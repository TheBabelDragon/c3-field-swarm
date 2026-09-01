#include "swarm/neighbor.h"
#include "swarm/state.h"

void neighbor_table_clear(NeighborTable& table) {
    table.count = 0;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        table.rows[i].node_id = 0;
        table.rows[i].boot_id = 0;
        table.rows[i].hardware_id = 0;
        table.rows[i].last_seen_ms = 0;
        table.rows[i].last_sequence = 0;
        table.rows[i].last_tick = 0;
        table.rows[i].rssi = 0;
        table.rows[i].capabilities = CAP_NONE;
        table.rows[i].state_version = 0;
        table.rows[i].alive = false;
    }
}

NeighborRecord* neighbor_find(NeighborTable& table, NodeId id) {
    for (uint8_t i = 0; i < table.count; ++i) {
        if (table.rows[i].node_id == id) {
            return &table.rows[i];
        }
    }
    return nullptr;
}

const NeighborRecord* neighbor_find_const(const NeighborTable& table, NodeId id) {
    for (uint8_t i = 0; i < table.count; ++i) {
        if (table.rows[i].node_id == id) {
            return &table.rows[i];
        }
    }
    return nullptr;
}

NeighborRecord* neighbor_upsert(NeighborTable& table, NodeId id) {
    NeighborRecord* existing = neighbor_find(table, id);
    if (existing != nullptr) {
        existing->alive = true;
        return existing;
    }
    if (table.count >= SWARM_MAX_NODES) {
        return nullptr;
    }
    NeighborRecord* row = &table.rows[table.count++];
    row->node_id = id;
    row->boot_id = 0;
    row->hardware_id = 0;
    row->last_seen_ms = 0;
    row->last_sequence = 0;
    row->last_tick = 0;
    row->rssi = 0;
    row->capabilities = CAP_COORD_ELIGIBLE;
    row->state_version = 0;
    row->alive = true;
    return row;
}

uint8_t neighbor_expire(NeighborTable& table, uint32_t now_ms, uint32_t timeout_ms) {
    uint8_t expired = 0;
    uint8_t w = 0;
    for (uint8_t i = 0; i < table.count; ++i) {
        uint32_t age = now_ms - table.rows[i].last_seen_ms;
        if (age > timeout_ms) {
            table.rows[i].alive = false;
            ++expired;
            continue;
        }
        table.rows[w++] = table.rows[i];
    }
    table.count = w;
    return expired;
}

uint8_t neighbor_alive_count(const NeighborTable& table) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < table.count; ++i) {
        if (table.rows[i].alive) {
            ++n;
        }
    }
    return n;
}

void neighbor_collect_ids(const NeighborTable& table, NodeId* out, uint8_t* count) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < table.count; ++i) {
        if (table.rows[i].alive) {
            out[n++] = table.rows[i].node_id;
        }
    }
    *count = n;
}
