#pragma once

#include "config.h"
#include "swarm/node_id.h"
#include "swarm/neighbor.h"

enum SwarmPhase : uint8_t {
    PHASE_BOOT = 0,
    PHASE_INITIALIZE,
    PHASE_DISCOVERY,
    PHASE_MEMBERSHIP,
    PHASE_ELECTION,
    PHASE_RUN,
};

struct Membership {
    uint64_t epoch;
    NodeId coordinator;
    NodeId members[SWARM_MAX_NODES];
    uint8_t count;
};

inline void membership_clear(Membership& m) {
    m.epoch = 0;
    m.coordinator = 0;
    m.count = 0;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        m.members[i] = 0;
    }
}

inline bool membership_contains(const Membership& m, NodeId id) {
    for (uint8_t i = 0; i < m.count; ++i) {
        if (m.members[i] == id) {
            return true;
        }
    }
    return false;
}

inline void membership_sort(Membership& m) {
    for (uint8_t i = 0; i < m.count; ++i) {
        for (uint8_t j = static_cast<uint8_t>(i + 1); j < m.count; ++j) {
            if (m.members[j] < m.members[i]) {
                NodeId tmp = m.members[i];
                m.members[i] = m.members[j];
                m.members[j] = tmp;
            }
        }
    }
}

inline bool membership_add(Membership& m, NodeId id) {
    if (membership_contains(m, id)) {
        return false;
    }
    if (m.count >= SWARM_MAX_NODES) {
        return false;
    }
    m.members[m.count++] = id;
    membership_sort(m);
    return true;
}

inline bool membership_remove(Membership& m, NodeId id) {
    uint8_t w = 0;
    bool removed = false;
    for (uint8_t i = 0; i < m.count; ++i) {
        if (m.members[i] == id) {
            removed = true;
            continue;
        }
        m.members[w++] = m.members[i];
    }
    m.count = w;
    return removed;
}

struct SwarmRuntime {
    NodeIdentity self;
    uint32_t capabilities;
    SwarmPhase phase;
    Membership membership;
    NeighborTable neighbors;
    NodeId coordinator;
    uint64_t sequence;
    uint64_t tick;
    uint32_t state_version;
    uint32_t last_hello_ms;
    uint32_t last_heartbeat_ms;
    uint32_t last_coord_seen_ms;
    bool coordinator_is_self;
};

#ifdef ARDUINO
void swarm_runtime_begin(SwarmRuntime& rt, const NodeIdentity& self, uint32_t caps);
#endif
