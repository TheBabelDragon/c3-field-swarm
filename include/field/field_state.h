#pragma once

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "swarm/protocol.h"
#include "swarm/node_id.h"

struct FieldState {
    float temperature;
    float information;
    float energy;
    float signal;
};

inline void field_state_zero(FieldState& s) {
    s.temperature = 0.0f;
    s.information = 0.0f;
    s.energy = 0.0f;
    s.signal = 0.0f;
}

inline float field_channel_get(const FieldState& s, Channel ch) {
    switch (ch) {
        case CH_TEMPERATURE: return s.temperature;
        case CH_INFORMATION: return s.information;
        case CH_ENERGY:      return s.energy;
        case CH_SIGNAL:      return s.signal;
        default:             return 0.0f;
    }
}

inline void field_channel_set(FieldState& s, Channel ch, float v) {
    switch (ch) {
        case CH_TEMPERATURE: s.temperature = v; break;
        case CH_INFORMATION: s.information = v; break;
        case CH_ENERGY:      s.energy = v; break;
        case CH_SIGNAL:      s.signal = v; break;
        default: break;
    }
}

inline uint32_t field_state_checksum(const FieldState& s, uint64_t tick, uint32_t version) {
    uint32_t acc = version ^ static_cast<uint32_t>(tick) ^ static_cast<uint32_t>(tick >> 32);
    const float* p = &s.temperature;
    for (int i = 0; i < 4; ++i) {
        uint32_t bits = 0;
        memcpy(&bits, &p[i], sizeof(uint32_t));
        acc ^= bits + static_cast<uint32_t>(i * 2654435761u);
        acc = (acc << 5) | (acc >> 27);
    }
    return acc;
}

struct NeighborField {
    NodeId node_id;
    FieldState state;
    uint32_t version;
    bool valid;
};

struct FieldStore {
    FieldState local;
    NeighborField neighbors[SWARM_MAX_NODES];
    uint8_t neighbor_count;
    uint32_t version;
};

void field_store_init(FieldStore& store);
void field_store_set_neighbor(FieldStore& store, NodeId id, const FieldState& state, uint32_t version);
void field_store_drop_neighbor(FieldStore& store, NodeId id);
void field_store_retain_members(FieldStore& store, const NodeId* members, uint8_t count);
