# test_flow_sensor.py
"""
Salvus serial helper ─ sniff or log JSON to CSV.

───────────────  QUICK START  ───────────────
MCU streams automatically:
    python test_flow_sensor.py -p COM6 -b 115200           # logs to CSV

Need a peek at raw bytes:
    python test_flow_sensor.py --sniff -p COM6 -b 115200   # view stream

Options:
  --term {lf|cr|crlf}   pick line terminator (default lf)
  -r  --flowrate N      send FLOWRATE=N (µL/min) before logging
  --nocmd               skip FLOWSTATE/FLOWRATE commands entirely
"""

import argparse, csv, json, sys, time
from datetime import datetime
from pathlib import Path
import serial

# ── defaults ─────────────────────────────────────────────────
DEFAULT_PORT, DEFAULT_BAUD = "COM6", 115200
DEFAULT_FLOW_RATE, DEFAULT_OUTFILE = 500, "output.csv"
PRINTABLE = bytes(range(0x20, 0x7F))           # ASCII 32-126 for sniffer

# ── helpers ──────────────────────────────────────────────────
def make_run_dir() -> Path:
    ts = datetime.now().strftime("%Y%m%d_%H%M_%S")
    p = Path(__file__).resolve().parent / ts
    p.mkdir(exist_ok=True)
    return p

def parse_term(name: str) -> bytes:
    name = name.lower()
    if name in ("lf", "n", "\\n"):      return b"\n"
    if name in ("cr", "r", "\\r"):      return b"\r"
    if name in ("crlf", "rn"):          return b"\r\n"
    sys.exit("Unknown --term (use lf, cr or crlf)")

# ── sniffer mode ─────────────────────────────────────────────
def sniff(port: str, baud: int):
    try:
        ser = serial.Serial(port, baud, timeout=1)
    except serial.SerialException as e:
        sys.exit(f"Cannot open {port}: {e}")
    ser.setDTR(True)             # ← keep line asserted so MCU streams
    ser.setRTS(False)
    time.sleep(0.1); ser.flushInput()

    print(f"Sniffing {port}@{baud} …  Ctrl-C to quit.\n")
    try:
        while True:
            data = ser.read(1024)
            if not data:
                continue
            ts = time.strftime("%H:%M:%S") + f".{int(time.time()*1e3)%1000:03d}"
            for b in data:
                ch = chr(b) if b in PRINTABLE else "·"
                print(f"{ts} {b:02X} {ch}")
    except KeyboardInterrupt:
        print("\nStopped.")

# ── logger mode ──────────────────────────────────────────────
def logger(port: str, baud: int, flow: int, send_cmds: bool,
           csv_path: Path, term: bytes):
    ser = serial.Serial(port, baud, timeout=1)
    ser.setDTR(True)             # assert DTR so CDC task starts
    ser.setRTS(False)
    time.sleep(0.1); ser.flushInput()

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
                if not raw_b:
                    continue
                raw = raw_b.decode("utf-8", errors="ignore").strip()
                print(raw)                       # console echo
                if not raw:
                    continue
                try:
                    rec = json.loads(raw)
                except json.JSONDecodeError:
                    print(f"[warn] malformed: {raw}", file=sys.stderr)
                    continue
                if writer is None:
                    writer = csv.DictWriter(fh, rec.keys(), extrasaction="ignore")
                    if fh.tell() == 0:
                        writer.writeheader()
                writer.writerow(rec); fh.flush()
        except KeyboardInterrupt:
            print("\nStopped.")

# ── CLI glue ─────────────────────────────────────────────────
def main():
    run = make_run_dir()
    ap = argparse.ArgumentParser(description="Salvus serial logger / sniffer")
    ap.add_argument("-p","--port", default=DEFAULT_PORT)
    ap.add_argument("-b","--baud", type=int, default=DEFAULT_BAUD)
    ap.add_argument("-r","--flowrate", type=int, default=DEFAULT_FLOW_RATE)
    ap.add_argument("-o","--outfile", default=DEFAULT_OUTFILE)
    ap.add_argument("--nocmd", action="store_true",
                    help="Don't send FLOWSTATE/FLOWRATE commands")
    ap.add_argument("--sniff", action="store_true", help="Sniffer mode")
    ap.add_argument("--term", default="lf",
                    help="Line terminator: lf (default), cr, crlf")
    args = ap.parse_args(); term = parse_term(args.term)

    if args.sniff:
        sniff(args.port, args.baud)
    else:
        csv_path = run / args.outfile
        logger(args.port, args.baud, args.flowrate,
               send_cmds=not args.nocmd, csv_path=csv_path, term=term)

if __name__ == "__main__":
    main()
