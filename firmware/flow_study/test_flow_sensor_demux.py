#!/usr/bin/env python3
"""
Salvus serial helper — DEMUX mode
Reads ONE serial port whose stream already contains {"id":…} JSON lines
and logs them all into a single CSV (id column included).

EXAMPLE
───────
python test_flow_sensor_demux.py -p COM6 -b 115200
"""
import argparse, csv, json, sys, time
from datetime import datetime
from pathlib import Path
import serial

# ── defaults ─────────────────────────────────────────────────
DEFAULT_PORT          = "COM6"
DEFAULT_BAUD          = 115200
DEFAULT_FLOW_RATE     = 500
DEFAULT_OUTFILE       = "output_demux.csv"
PRINTABLE             = bytes(range(0x20, 0x7F))

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

# ── logger core ──────────────────────────────────────────────
def logger(port:str, baud:int, flow:int, send_cmds:bool,
           csv_path:Path, term:bytes):

    try:
        ser = serial.Serial(port, baud, timeout=1)
    except serial.SerialException as e:
        sys.exit(f"[{port}] Cannot open: {e}")

    # keep MCU happy
    ser.setDTR(True); ser.setRTS(False)
    time.sleep(0.1);  ser.flushInput()

    # optional startup commands
    if send_cmds:
        for cmd in (f"FLOWSTATE=ON{term.decode()}",
                    f"FLOWRATE={flow}{term.decode()}"):
            ser.write(cmd.encode()); ser.flush()
            print(f"[sent] {cmd.strip()}"); time.sleep(0.05)

    writer = None
    with csv_path.open("a", newline="") as fh:
        print(f"Logging → {csv_path}  (Ctrl-C to stop)")
        try:
            while True:
                raw_b = ser.read_until(term)
                if not raw_b:                       # nothing yet
                    continue
                raw = raw_b.decode("utf-8", errors="ignore").strip()
                if raw:
                    print(raw)                      # console echo
                else:
                    continue
                try:
                    rec = json.loads(raw)
                except json.JSONDecodeError:
                    print(f"[warn] malformed: {raw}", file=sys.stderr)
                    continue
                # CSV header on first valid record
                if writer is None:
                    writer = csv.DictWriter(fh, rec.keys(), extrasaction="ignore")
                    if fh.tell() == 0:              # empty file
                        writer.writeheader()
                writer.writerow(rec); fh.flush()
        except KeyboardInterrupt:
            print("\nStopped.")

# ── CLI glue ─────────────────────────────────────────────────
def main():
    run = make_run_dir()
    ap  = argparse.ArgumentParser(description="Salvus serial demux logger")
    ap.add_argument("-p","--port", default=DEFAULT_PORT,
                    help="Single serial port (default COM6)")
    ap.add_argument("-b","--baud", type=int, default=DEFAULT_BAUD)
    ap.add_argument("-r","--flowrate", type=int, default=DEFAULT_FLOW_RATE)
    ap.add_argument("-o","--outfile", default=DEFAULT_OUTFILE,
                    help="CSV name (default output_demux.csv)")
    ap.add_argument("--nocmd", action="store_true",
                    help="Don't send FLOWSTATE/FLOWRATE commands")
    ap.add_argument("--term", default="lf",
                    help="Line terminator: lf (default), cr, crlf")
    args   = ap.parse_args()
    term_b = parse_term(args.term)

    csv_path = run / args.outfile
    logger(args.port, args.baud, args.flowrate,
           send_cmds=not args.nocmd, csv_path=csv_path, term=term_b)

if __name__ == "__main__":
    main()
