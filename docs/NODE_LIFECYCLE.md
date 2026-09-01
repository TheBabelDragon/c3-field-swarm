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

A new or rebooted node broadcasts HELLO. Peers ACK and insert it into the neighbor table. If a coordinator already exists, the joiner accepts that coordinator and current membership epoch.

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
