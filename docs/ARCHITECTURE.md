# Architecture

The six ESP32-C3 Super Mini boards are six instances of one machine.

```
┌─────────────────────────────────────────────┐
│                 Applications                │
│       sensors / LEDs / motors / etc.        │
├─────────────────────────────────────────────┤
│                  Systems                    │
│ diffusion / decay / aggregation / rules     │
├─────────────────────────────────────────────┤
│                 Field State                 │
│ local state / neighbor state / tick state   │
├─────────────────────────────────────────────┤
│                 Swarm Core                  │
│ discovery / heartbeat / gossip / election   │
├─────────────────────────────────────────────┤
│                Transport                    │
│                 ESP-NOW                     │
├─────────────────────────────────────────────┤
│                ESP32-C3                     │
└─────────────────────────────────────────────┘
```

## Layer rules

- Transport does not know about sensors.
- Swarm core does not know about LEDs.
- Field layer does not know about ESP-NOW.
- Application layer does not manipulate swarm membership directly.
- Systems receive a frozen `FieldView` and emit `FieldDelta` values. They never write shared state.

## Identities

| Name | Lifetime | Purpose |
| --- | --- | --- |
| `hardware_id` | factory MAC | permanent physical identity |
| `node_id` | persisted NVS | logical identity in the current swarm |
| `boot_id` | one reboot | distinguishes same node, new boot vs stale packet |

Packets carry `sender_id`, `boot_id`, and `sequence` so a reboot is not confused with an old datagram.

## Time

Three clocks stay separate:

- wall clock (`millis`) for timeouts and serial UX
- network arrival time, which is nondeterministic
- field tick (`tick + dt`) for system evaluation

Coordinator proposes `FIELD_TICK`. Followers apply it. If the coordinator disappears, nodes continue locally with bounded loss rather than freezing.

## Repository boundary

This repository is the wireless edge substrate.

- `field-bus` remains the CAN-FD physical-node protocol.
- `metafield-engine` remains the simulation / substrate engine.
- A future `metafield_bridge/` may translate C3 field state into MetaField / field-bus. That bridge is not part of milestone 1.

## Future adapters

```
class Sensor { virtual void sample(SensorFrame&) = 0; };
class Actuator { virtual void apply(const ActuatorCommand&) = 0; };
```

No sensor or actuator is compiled into the core.
