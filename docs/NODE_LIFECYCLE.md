# Node lifecycle

```
BOOT
  ↓
INITIALIZE     identity from MAC + NVS, new boot_id
  ↓
BROADCAST HELLO
  ↓
COLLECT NEIGHBORS
  ↓
ESTABLISH MEMBERSHIP
  ↓
ELECTION       only if no live coordinator
  ↓
RUN            heartbeats, ticks, systems, gossip
```

## Join

A newly flashed board needs **no serial config**. Empty NVS is a valid start.

1. Derive `hardware_id` from the factory MAC. Restore `node_id` from NVS, or pick `1..6` from the MAC and persist it.
2. Stay in discovery for `SWARM_DISCOVERY_MS` (~2.25s): broadcast HELLO, collect neighbors, **do not elect**.
3. If a live coordinator is heard, adopt it and its membership epoch. A lone node that already self-elected still yields to that swarm.
4. If nobody is coordinating when discovery ends, elect among the nodes actually seen (lone board becomes coordinator).
5. If two boards claim the same `node_id` (MAC hash collision), the lower `hardware_id` keeps it. The other persists the next free id `1..6` and HELLO again.

`id 1..6` on the console is only for desk labels. It is not required to join.

Reflash without erasing NVS keeps the same logical id. Erase flash / empty NVS still joins.

## Leave

Timeout expires the neighbor (`SWARM_NEIGHBOR_TIMEOUT_MS`). GOODBYE is the explicit path. Membership epoch increments.

## Coordinator failure

```
timeout
  ↓
election
  ↓
highest eligible live node
  ↓
new coordinator
  ↓
membership epoch++
```

Power off NODE 01. The other five converge. Power NODE 01 back on. It rejoins. It does not automatically reclaim leadership.

## Field execution

```
current state
     ↓
frozen view
     ↓
systems evaluate
     ↓
FieldDelta[]
     ↓
validate
     ↓
sort
     ↓
apply
     ↓
new state
```

## Serial console

```
help
status
nodes
state
tick
neighbors
elect
inject <temp|info|energy|signal> <value>
id <1-6>
reset
```

## Bring-up stages

1. One C3 — identity, boot_id, heartbeat, console, field tick
2. Two C3s — discovery, HELLO, neighbor table, state exchange
3. Three C3s — gossip, diffusion, deterministic tick
4. Six C3s — full membership, election, field propagation
5. Kill coordinator — automatic election, epoch bump, field continues
6. Reboot a random node — rejoin, sync, no duplicate identity
7. Remove two nodes — remaining swarm continues
8. Restore both — automatic convergence
