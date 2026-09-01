#include "field/field_state.h"

void field_store_init(FieldStore& store) {
    field_state_zero(store.local);
    store.neighbor_count = 0;
    store.version = 1;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        store.neighbors[i].node_id = 0;
        field_state_zero(store.neighbors[i].state);
        store.neighbors[i].version = 0;
        store.neighbors[i].valid = false;
    }
}

void field_store_set_neighbor(FieldStore& store, NodeId id, const FieldState& state, uint32_t version) {
    for (uint8_t i = 0; i < store.neighbor_count; ++i) {
        if (store.neighbors[i].node_id == id) {
            store.neighbors[i].state = state;
            store.neighbors[i].version = version;
            store.neighbors[i].valid = true;
            return;
        }
    }
    if (store.neighbor_count >= SWARM_MAX_NODES) {
        return;
    }
    uint8_t i = store.neighbor_count++;
    store.neighbors[i].node_id = id;
    store.neighbors[i].state = state;
    store.neighbors[i].version = version;
    store.neighbors[i].valid = true;

    for (uint8_t a = 0; a < store.neighbor_count; ++a) {
        for (uint8_t b = static_cast<uint8_t>(a + 1); b < store.neighbor_count; ++b) {
            if (store.neighbors[b].node_id < store.neighbors[a].node_id) {
                NeighborField tmp = store.neighbors[a];
                store.neighbors[a] = store.neighbors[b];
                store.neighbors[b] = tmp;
            }
        }
    }
}

void field_store_drop_neighbor(FieldStore& store, NodeId id) {
    uint8_t w = 0;
    for (uint8_t i = 0; i < store.neighbor_count; ++i) {
        if (store.neighbors[i].node_id == id) {
            continue;
        }
        store.neighbors[w++] = store.neighbors[i];
    }
    store.neighbor_count = w;
}

void field_store_retain_members(FieldStore& store, const NodeId* members, uint8_t count) {
    uint8_t w = 0;
    for (uint8_t i = 0; i < store.neighbor_count; ++i) {
        bool keep = false;
        for (uint8_t m = 0; m < count; ++m) {
            if (members[m] == store.neighbors[i].node_id) {
                keep = true;
                break;
            }
        }
        if (keep) {
            store.neighbors[w++] = store.neighbors[i];
        }
    }
    store.neighbor_count = w;
}
