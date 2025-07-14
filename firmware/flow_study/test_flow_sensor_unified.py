#!/usr/bin/env python3
"""
Salvus serial helper — MULTI-PORT, PER-ID FILES, QUIET LOGGING
Stop the experiment by pressing the chosen key + Enter (default 'q').
"""

import argparse, csv, json, sys, time, threading, queue, os
from datetime import datetime
from pathlib import Path
import serial

# ── defaults ─────────────────────────────────────────────────
DEFAULT_PORTS      = ["COM6"]
DEFAULT_BAUD       = 115200
DEFAULT_FLOW_RATE  = 500
UNIFIED_OUTFILE    = "output_unified.csv"          # None = skip
PRINTABLE          = bytes(range(0x20, 0x7F))

# ── helpers ──────────────────────────────────────────────────
def make_run_dir() -> Path:
    ts = datetime.now().strftime("%Y%m%d_%H%M_%S")
    p  = Path(__file__).resolve().parent / ts
    p.mkdir(exist_ok=True)
    return p

def parse_term(name:str) -> bytes:
    name = name.lower()
    if name in ("lf","n","\\n"):      return b"\n"
    if name in ("cr","r","\\r"):      return b"\r"
    if name in ("crlf","rn"):         return b"\r\n"
    sys.exit("Unknown --term (lf, cr, crlf)")

# ── CSV writers & open ports ─────────────────────────────────
writers_by_id, writers_lk = {}, threading.Lock()
unified_writer = unified_fh = None
unified_lk     = threading.Lock()
open_ports     = []                        # list[serial.Serial]

def write_row_per_id(rec, folder):
    dev_id = rec.get("id", "unknown")
    fname  = folder / f"device_{dev_id}.csv"
    with writers_lk:
        writer, fh = writers_by_id.get(dev_id, (None, None))
        if writer is None:
            fh = fname.open("a", newline="")
            writer = csv.DictWriter(fh, rec.keys(), extrasaction="ignore")
            if fh.tell() == 0:
                writer.writeheader()
            writers_by_id[dev_id] = (writer, fh)
        writer.writerow(rec); fh.flush()

def write_row_unified(rec, folder):
    if UNIFIED_OUTFILE is None:
        return
    global unified_writer, unified_fh
    if unified_writer is None:
        unified_fh = (folder / UNIFIED_OUTFILE).open("a", newline="")
        unified_writer = csv.DictWriter(unified_fh, rec.keys(),
                                        extrasaction="ignore")
        if unified_fh.tell() == 0:
            unified_writer.writeheader()
    with unified_lk:
        unified_writer.writerow(rec); unified_fh.flush()

# ── serial worker ────────────────────────────────────────────
def worker(port, baud, flow, send_cmds, term, folder, quiet):
    try:
        ser = serial.Serial(port, baud, timeout=1)
    except serial.SerialException as e:
        sys.exit(f"[{port}] Cannot open: {e}")
    open_ports.append(ser)

    ser.setDTR(True); ser.setRTS(False)
    time.sleep(0.1); ser.flushInput()

    if send_cmds:
        for cmd in (f"FLOWSTATE=ON{term.decode()}",
                    f"FLOWRATE={flow}{term.decode()}"):
            ser.write(cmd.encode()); ser.flush()
            if not quiet:
                print(f"[{port}] [sent] {cmd.strip()}")

    if not quiet:
        print(f"[{port}] Logging… (press stop key in main window)")
    try:
        while True:
            raw_b = ser.read_until(term)
            if not raw_b:
                continue
            raw = raw_b.decode("utf-8", errors="ignore").strip()
            if not raw:
                continue
            if not quiet:
                print(f"[{port}] {raw}")
            try:
                rec = json.loads(raw)
            except json.JSONDecodeError:
                if not quiet:
                    print(f"[{port}] [warn] malformed JSON")
                continue
            rec["port"] = port
            write_row_per_id(rec, folder)
            write_row_unified(rec, folder)
    except (KeyboardInterrupt, SystemExit):
        pass  # graceful shutdown in main()

# ── stdin watcher ────────────────────────────────────────────
def stdin_watcher(stop_queue: queue.Queue, stop_char: str):
    """
    Wait for <stop_char>\n on stdin and notify main thread through queue.
    """
    while True:
        try:
            line = input()
        except EOFError:
            break
        if line.strip().lower() == stop_char:
            stop_queue.put("STOP")
            break

# ── CLI glue ─────────────────────────────────────────────────
def main():
    run = make_run_dir()
    ap = argparse.ArgumentParser(description="Salvus multi-port logger")
    ap.add_argument("-p","--port", nargs="+", default=DEFAULT_PORTS)
    ap.add_argument("-b","--baud", type=int, default=DEFAULT_BAUD)
    ap.add_argument("-r","--flowrate", type=int, default=DEFAULT_FLOW_RATE)
    ap.add_argument("--nocmd", action="store_true",
                    help="Skip FLOWSTATE/FLOWRATE commands")
    ap.add_argument("--term", default="lf")
    ap.add_argument("--stop", metavar="CMD", default="FLOWSTATE=OFF",
                    help="Command broadcast on shutdown (default FLOWSTATE=OFF)")
    ap.add_argument("--keystop", default="q",
                    help="Key + Enter to stop (default 'q')")
    ap.add_argument("--quiet", action="store_true",
                    help="Suppress per-line console echo")
    args = ap.parse_args()
    term_b = parse_term(args.term)

    if not args.quiet:
        print(f"Run folder: {run}")
        print(f"Press '{args.keystop}' then Enter to stop.")
        if UNIFIED_OUTFILE:
            print(f"(Also logging to {UNIFIED_OUTFILE})")

    # spawn stdin watcher
    stop_q = queue.Queue()
    threading.Thread(target=stdin_watcher,
                     args=(stop_q, args.keystop.lower()),
                     daemon=True).start()

    # spawn serial workers
    threads=[]
    for port in args.port:
        th = threading.Thread(target=worker,
                              args=(port, args.baud, args.flowrate,
                                    not args.nocmd, term_b, run, args.quiet),
                              daemon=True)
        th.start(); threads.append(th)

    try:
        while True:
            for th in threads:
                th.join(timeout=0.2)       # keep loop reactive
            if not stop_q.empty():
                raise KeyboardInterrupt
    except KeyboardInterrupt:
        if not args.quiet:
            print(f"\n[SHUTDOWN] Broadcasting '{args.stop}'…")
        stop_line = (args.stop + term_b.decode()).encode()
        for ser in open_ports:
            try:
                ser.write(stop_line); ser.flush()
                if not args.quiet:
                    print(f"  → {ser.port}")
            except Exception as e:
                print(f"  ! {ser.port}: {e}", file=sys.stderr)
        time.sleep(0.05)
    finally:
        for ser in open_ports:
            ser.close()
        if not args.quiet:
            print("Stopped.")

if __name__ == "__main__":
    main()
