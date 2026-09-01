#include "swarm/protocol.h"
#include "swarm/state.h"

void heartbeat_fill(HeartbeatPayload& p,
                    const SwarmRuntime& rt,
                    uint32_t uptime_ms,
                    uint32_t free_heap) {
    p.node_id = rt.self.node_id;
    p.boot_id = rt.self.boot_id;
    p.tick = rt.tick;
    p.uptime_ms = uptime_ms;
    p.free_heap = free_heap;
    p.local_state_version = rt.state_version;
    p.coordinator_id = rt.coordinator;
}
