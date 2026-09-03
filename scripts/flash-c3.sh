#!/usr/bin/env bash
# Reflash every plugged-in ESP32-C3 and give each a unique node_id.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
if ! command -v pio >/dev/null 2>&1; then
  echo "platformio missing — pipx install platformio"
  exit 1
fi
shopt -s nullglob
ports=(/dev/ttyACM* /dev/ttyUSB*)
if [[ ${#ports[@]} -eq 0 ]]; then
  echo "no /dev/ttyACM* or /dev/ttyUSB* — unplug the CYD, plug C3s"
  exit 1
fi
echo "flashing ${#ports[@]} C3(s): ${ports[*]}"
n=1
for p in "${ports[@]}"; do
  echo
  echo "==== upload $p as node $n ===="
  pio run -e esp32-c3-supermini -t upload --upload-port "$p"
  python3 - "$p" "$n" <<'PY'
import sys, time
port, nid = sys.argv[1], sys.argv[2]
try:
    import serial
except ImportError:
    print("pyserial missing — sudo pacman -S --needed python-pyserial")
    raise SystemExit(1)
s = serial.Serial()
s.port = port
s.baudrate = 115200
s.timeout = 0.3
s.dsrdtr = False
s.rtscts = False
s.dtr = False
s.rts = False
s.open()
time.sleep(1.5)
s.write(f"id {nid}\n".encode())
s.flush()
time.sleep(0.4)
print(f"assigned node_id={int(nid):02d} on {port} (board will reboot)")
s.close()
PY
  n=$((n+1))
done
echo
echo "leave ONE C3 plugged into the host. Power the other three on the desk."
echo "then: cd ~/metafield-engine && git pull origin main && bash scripts/run-arch.sh"
