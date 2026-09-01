#include "swarm/state.h"
#include "swarm/protocol.h"

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
