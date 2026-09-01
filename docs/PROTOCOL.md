# Protocol

Versioned binary packets over ESP-NOW. No router, internet, MQTT, or cloud.

Header (28 bytes, little-endian):

```
struct PacketHeader {
    uint8_t  version;
    uint8_t  type;
    uint16_t length;      // total bytes including header
    uint32_t sender_id;   // logical node_id
    uint32_t boot_id;
    uint64_t sequence;
    uint64_t tick;
};
```

Unknown `version` values are rejected. `length` is never trusted as a raw memcpy size.

## Types

| Type | Name | Role |
| --- | --- | --- |
| 1 | HELLO | advertise identity + capabilities |
| 2 | HELLO_ACK | unicast reply |
| 3 | HEARTBEAT | 500 ms liveness + coordinator hint |
| 4 | MEMBERSHIP | epoch, coordinator, member list |
| 5 | STATE_DIGEST | cheap divergence detector |
| 6 | STATE_DELTA | bounded field snapshot |
| 7 | FIELD_TICK | coordinator tick proposal |
| 8 | COMMAND | host / peer command |
| 9 | COMMAND_ACK | command result |
| 10 | ELECTION | candidate announcement |
| 11 | ELECTION_ACK | election echo |
| 12 | TELEMETRY | optional binary telemetry |
| 13 | GOODBYE | explicit leave |
| 14 | STATE_REQUEST | request a STATE_DELTA |

## HELLO body

`hardware_id`, `node_id`, `boot_id`, protocol version, firmware version, capabilities.

## HEARTBEAT body

`node_id`, `boot_id`, `tick`, `uptime_ms`, `free_heap`, `local_state_version`, `coordinator_id`.

Period: 500 ms (same cadence convention as other MetaField physical nodes; this wireless protocol is still independent).

## MEMBERSHIP body

`epoch`, `coordinator`, `count`, `members[MAX_NODES]`.

A packet whose `epoch` is older than the locally observed epoch is ignored.

## Election

First implementation: highest eligible `node_id` wins. Tie-break: `boot_id`.

Rejoin of a higher `node_id` does **not** automatically reclaim leadership. Election runs on coordinator timeout, empty coordinator, or an explicit `elect` command.

## Field snapshot

For the six-node prototype a bounded full-state `STATE_DELTA` is acceptable. Optimize after measuring.

## Fault injection

Compile with `-DSWARM_FAULT_INJECTION=1` to simulate drop / duplicate / reorder. Network nondeterminism must not become simulation nondeterminism.
