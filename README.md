# c3-field-swarm

Deterministic ESP32-C3 wireless swarm substrate for the MetaField physical field.

Six interchangeable low-cost nodes discover, synchronize, gossip state, elect a coordinator, and execute deterministic local field systems. Firmware is identical on every board. Roles are runtime state, not soldered identity.

```
C3 swarm protocol
        ↓   (later)
MetaField bridge
        ↓
field-bus / CAN-FD
```

This repository does **not** implement the CAN-FD protocol or the MetaField engine. Those stay in `field-bus` and `metafield-engine`.

## Why this repo exists

`field-bus` is the CAN-FD physical-node contract. Turning it into another firmware tree would blur that contract. The C3 fleet is a wireless edge substrate with its own protocol and the same frozen-view / FieldDelta discipline as the larger MetaField stack.

## Build

```
pio run
pio run -t upload
pio device monitor
```

Pinned platform: `espressif32@6.9.0`, Arduino framework, board `esp32-c3-devkitm-1` (ESP32-C3 Super Mini).

Host tests (no hardware):

```
pio test -e native
```

or

```
bash scripts/run_host_tests.sh
```

## First boot

Flash the same firmware onto a blank C3 and power it next to the fleet. It will discover on ESP-NOW channel 1, adopt a live coordinator if one exists, and persist a logical `node_id` in NVS. No SSID, no serial `id`, no prior config.

Every board derives `hardware_id` from the factory MAC, persists `node_id` (created from the MAC on first boot, kept after reflash unless NVS is erased), and generates a new `boot_id` each reboot. Duplicate logical ids are resolved: the lower MAC keeps the number, the other persists the next free slot.

Optional desk labels 01–06:

```
> id 1
```

Do not encode `01 = coordinator forever`. Highest eligible live `node_id` wins an election; a rejoin does not automatically reclaim leadership.

## Serial

```
> help
> status
> nodes
> state
> tick
> neighbors
> elect
> inject info 1.0
> reset
```

## Host tool

```
python3 tools/swarmctl/swarmctl.py discover
python3 tools/swarmctl/swarmctl.py --port /dev/ttyACM0 status
```

Not required for the fleet to run.

## Docs

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- [docs/PROTOCOL.md](docs/PROTOCOL.md)
- [docs/NODE_LIFECYCLE.md](docs/NODE_LIFECYCLE.md)
- [docs/HARDWARE.md](docs/HARDWARE.md)
