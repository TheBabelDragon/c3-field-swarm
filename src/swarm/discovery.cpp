#include "swarm/state.h"
#include "swarm/protocol.h"
#include "swarm/join.h"

bool join_should_adopt(NodeId self_id,
                       uint8_t self_members,
                       uint64_t self_epoch,
                       NodeId self_coord,
                       bool in_discovery,
                       NodeId foreign_coord,
                       uint8_t foreign_members,
                       uint64_t foreign_epoch) {
    if (foreign_coord == 0 || foreign_coord == self_id) {
        return false;
    }
    if (in_discovery) {
        return true;
    }
    if (self_coord == 0) {
        return true;
    }
    // A lone node that already self-elected still yields to a live swarm.
    if (self_members <= 1) {
        return true;
    }
    if (foreign_epoch > self_epoch) {
        return true;
    }
    if (foreign_epoch == self_epoch && foreign_members > self_members) {
        return true;
    }
    return false;
}

bool join_should_yield_node_id(HardwareId self_hw, HardwareId other_hw) {
    if (other_hw == 0 || other_hw == self_hw) {
        return false;
    }
    return self_hw > other_hw;
}

NodeId join_next_free_id(const NodeId* taken, uint8_t taken_count) {
    for (NodeId id = 1; id <= SWARM_MAX_NODES; ++id) {
        bool used = false;
        for (uint8_t i = 0; i < taken_count; ++i) {
            if (taken[i] == id) {
                used = true;
                break;
            }
        }
        if (!used) {
            return id;
        }
    }
    return 0;
}

#ifdef ARDUINO

void swarm_runtime_begin(SwarmRuntime& rt, const NodeIdentity& self, uint32_t caps) {
    rt.self = self;
    rt.capabilities = caps;
    rt.phase = PHASE_BOOT;
    membership_clear(rt.membership);
    neighbor_table_clear(rt.neighbors);
    membership_add(rt.membership, self.node_id);
    rt.membership.epoch = 1;
    rt.coordinator = 0;
    rt.sequence = 1;
    rt.tick = 0;
    rt.state_version = 1;
    rt.last_hello_ms = 0;
    rt.last_heartbeat_ms = 0;
    rt.last_coord_seen_ms = 0;
    rt.coordinator_is_self = false;
    rt.phase = PHASE_INITIALIZE;
}

#endif
