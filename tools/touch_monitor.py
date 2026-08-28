"""
touch_monitor.py  —  Luna Touch Zone Serial Monitor
Reads COM3 and prints LEFT / CENTER / RIGHT / LONG clearly in color.
"""
import sys, re, serial, io

# Force UTF-8 on Windows terminal so colour codes render correctly
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")


PORT = "COM3"
BAUD = 115200

# ANSI colors
RED    = "\033[91m"
GREEN  = "\033[92m"
BLUE   = "\033[94m"
PURPLE = "\033[95m"
CYAN   = "\033[96m"
YELLOW = "\033[93m"
GRAY   = "\033[90m"
BOLD   = "\033[1m"
RESET  = "\033[0m"

# Patterns from interaction.h serial output
ZONE_RE = re.compile(r'\[Touch\]\s+(LEFT|CENTER|RIGHT)\s+tap\s+\(X=(\d+)\)', re.IGNORECASE)
LONG_RE = re.compile(r'\[Touch\]\s+Long\s+press', re.IGNORECASE)
RAW_RE  = re.compile(r'\[TOUCH\]\s+G:', re.IGNORECASE)

ZONE_CFG = {
    "LEFT":   (RED,    "<<< LEFT  <<<", "-> Prev Screen"),
    "CENTER": (GREEN,  " *  CENTER  *", "-> Select / Interact"),
    "RIGHT":  (BLUE,   ">>> RIGHT >>>", "-> Next Screen"),
    "LONG":   (PURPLE, "[LONG PRESS]  ", "-> Home Screen"),
}

def banner():
    print(f"\n{BOLD}{CYAN}{'='*54}{RESET}")
    print(f"{BOLD}{CYAN}  Luna Touch Zone Monitor  --  {PORT} @ {BAUD}{RESET}")
    print(f"{BOLD}{CYAN}{'='*54}{RESET}")
    print(f"{GRAY}  Zones: X<80 = LEFT | X80-159 = CENTER | X>=160 = RIGHT{RESET}")
    print(f"{GRAY}  Long press (>=500ms) = HOME{RESET}")
    print(f"{BOLD}{CYAN}{'-'*54}{RESET}\n")

def main():
    banner()
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        print(f"{GREEN}✓ Connected to {PORT}{RESET}\n")
    except Exception as e:
        print(f"{RED}✗ Cannot open {PORT}: {e}{RESET}")
        sys.exit(1)

    tap_count = {"LEFT": 0, "CENTER": 0, "RIGHT": 0, "LONG": 0}

    try:
        while True:
            raw = ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            # ── Zone tap ──────────────────────────────────────────────────────
            m = ZONE_RE.search(line)
            if m:
                zone = m.group(1).upper()
                x    = int(m.group(2))
                col, label, action = ZONE_CFG[zone]
                tap_count[zone] += 1
                print(f"{BOLD}{col}  {label}   {action}   X={x}px{RESET}"
                      f"  {GRAY}(#{tap_count[zone]}){RESET}")
                continue

            # ── Long press ───────────────────────────────────────────────────
            if LONG_RE.search(line):
                col, label, action = ZONE_CFG["LONG"]
                tap_count["LONG"] += 1
                print(f"{BOLD}{col}  {label}  {action}{RESET}"
                      f"  {GRAY}(#{tap_count['LONG']}){RESET}")
                continue

            # ── Raw touch (dim) ──────────────────────────────────────────────
            if RAW_RE.search(line):
                print(f"  {GRAY}{line}{RESET}")
                continue

            # ── Other firmware output ────────────────────────────────────────
            if line:
                print(f"  {GRAY}{line}{RESET}")

    except KeyboardInterrupt:
        print(f"\n{YELLOW}Stopped.{RESET}")
        total = sum(tap_count.values())
        print(f"\n{BOLD}Session Summary:{RESET}")
        for zone, count in tap_count.items():
            col = ZONE_CFG[zone][0]
            print(f"  {col}{zone:8s}{RESET}  {count} taps")
        print(f"  {'TOTAL':8s}  {total} taps\n")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
