#pragma once

#include <stdint.h>

#include "config.h"
#include "swarm/node_id.h"
#include "swarm/capabilities.h"

struct NeighborRecord {
    NodeId node_id;
    BootId boot_id;
    HardwareId hardware_id;
    uint32_t last_seen_ms;
    uint64_t last_sequence;
    uint64_t last_tick;
    int8_t rssi;
    uint32_t capabilities;
    uint32_t state_version;
    bool alive;
};

struct NeighborTable {
    NeighborRecord rows[SWARM_MAX_NODES];
    uint8_t count;
};

void neighbor_table_clear(NeighborTable& table);
NeighborRecord* neighbor_find(NeighborTable& table, NodeId id);
const NeighborRecord* neighbor_find_const(const NeighborTable& table, NodeId id);
NeighborRecord* neighbor_upsert(NeighborTable& table, NodeId id);
uint8_t neighbor_expire(NeighborTable& table, uint32_t now_ms, uint32_t timeout_ms);
uint8_t neighbor_alive_count(const NeighborTable& table);
void neighbor_collect_ids(const NeighborTable& table, NodeId* out, uint8_t* count);
