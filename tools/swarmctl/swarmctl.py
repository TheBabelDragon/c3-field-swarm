#!/usr/bin/env python3
"""Host-side operator tool for c3-field-swarm."""

from __future__ import annotations

import argparse
import glob
import sys
import time


def find_ports() -> list[str]:
    patterns = [
        "/dev/ttyACM*",
        "/dev/ttyUSB*",
        "/dev/cu.usbmodem*",
        "/dev/cu.usbserial*",
        "/dev/cu.wchusbserial*",
    ]
    ports: list[str] = []
    for pat in patterns:
        ports.extend(sorted(glob.glob(pat)))
    return ports


def open_serial(port: str | None, baud: int = 115200):
    try:
        import serial  # type: ignore
    except ImportError as exc:
        raise SystemExit("pyserial is required: pip install pyserial") from exc
    if port is None:
        ports = find_ports()
        if not ports:
            raise SystemExit("no serial ports found")
        port = ports[0]
    ser = serial.Serial(port, baud, timeout=0.4)
    time.sleep(0.2)
    return ser


def send_command(ser, command: str, settle: float = 0.25) -> str:
    ser.reset_input_buffer()
    ser.write((command + "\n").encode("utf-8"))
    ser.flush()
    time.sleep(settle)
    chunks: list[bytes] = []
    deadline = time.time() + 0.6
    while time.time() < deadline:
        waiting = ser.in_waiting
        if waiting:
            chunks.append(ser.read(waiting))
        else:
            time.sleep(0.05)
    return b"".join(chunks).decode("utf-8", errors="replace")


def cmd_discover(args: argparse.Namespace) -> int:
    ports = find_ports()
    if not ports:
        print("no serial devices")
        return 1
    print("candidate ports:")
    for p in ports:
        print(f"  {p}")
        if args.probe:
            try:
                ser = open_serial(p, args.baud)
                out = send_command(ser, "status")
                ser.close()
                for line in out.splitlines():
                    if line.startswith("[C3]") or line.startswith("node"):
                        print(f"    {line}")
            except Exception as exc:  # noqa: BLE001
                print(f"    probe failed: {exc}")
    return 0


def cmd_serial(args: argparse.Namespace, line: str) -> int:
    ser = open_serial(args.port, args.baud)
    try:
        print(send_command(ser, line), end="")
    finally:
        ser.close()
    return 0


def cmd_logs(args: argparse.Namespace) -> int:
    ser = open_serial(args.port, args.baud)
    print(f"listening on {ser.port}  ctrl-c to stop", file=sys.stderr)
    try:
        while True:
            raw = ser.readline()
            if raw:
                sys.stdout.write(raw.decode("utf-8", errors="replace"))
                sys.stdout.flush()
    except KeyboardInterrupt:
        return 0
    finally:
        ser.close()


def main() -> int:
    parser = argparse.ArgumentParser(prog="swarmctl", description="c3-field-swarm host tool")
    parser.add_argument("--port", default=None, help="serial device")
    parser.add_argument("--baud", type=int, default=115200)
    sub = parser.add_subparsers(dest="cmd", required=True)
    d = sub.add_parser("discover", help="list candidate serial ports")
    d.add_argument("--probe", action="store_true")
    sub.add_parser("status")
    sub.add_parser("nodes")
    sub.add_parser("state")
    sub.add_parser("tick")
    sub.add_parser("logs")
    inj = sub.add_parser("inject")
    inj.add_argument("channel", choices=["temp", "info", "energy", "signal"])
    inj.add_argument("value", type=float)
    args = parser.parse_args()
    if args.cmd == "discover":
        return cmd_discover(args)
    if args.cmd == "logs":
        return cmd_logs(args)
    if args.cmd == "inject":
        return cmd_serial(args, f"inject {args.channel} {args.value}")
    return cmd_serial(args, args.cmd)


if __name__ == "__main__":
    raise SystemExit(main())
