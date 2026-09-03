#!/usr/bin/env bash
# Reflash every plugged-in ESP32-C3 with c3-field-swarm (C3JSON on boot).
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
  echo "no /dev/ttyACM* or /dev/ttyUSB* — plug a C3 USB"
  exit 1
fi
echo "flashing ${#ports[@]} port(s): ${ports[*]}"
for p in "${ports[@]}"; do
  echo
  echo "==== upload $p ===="
  pio run -e esp32-c3-supermini -t upload --upload-port "$p"
done
echo
echo "leave one C3 plugged in, then:"
echo "  cd ~/metafield-engine && git pull origin main && bash scripts/run-arch.sh"
