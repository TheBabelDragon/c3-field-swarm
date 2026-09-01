#include "swarm/election.h"

ElectionResult elect_highest_eligible(const NodeId* ids,
                                      const BootId* boots,
                                      const uint32_t* capabilities,
                                      uint8_t count) {
    ElectionResult r;
    r.winner = 0;
    r.winner_boot = 0;
    r.valid = false;

    for (uint8_t i = 0; i < count; ++i) {
        if (ids[i] == 0) {
            continue;
        }
        if (capabilities != nullptr && !has_capability(capabilities[i], CAP_COORD_ELIGIBLE)) {
            continue;
        }
        if (!r.valid || ids[i] > r.winner ||
            (ids[i] == r.winner && boots[i] > r.winner_boot)) {
            r.winner = ids[i];
            r.winner_boot = boots[i];
            r.valid = true;
        }
    }
    return r;
}

ElectionResult elect_from_membership(const Membership& membership,
                                     const NeighborTable& neighbors,
                                     const NodeIdentity& self,
                                     uint32_t self_caps) {
    NodeId ids[SWARM_MAX_NODES];
    BootId boots[SWARM_MAX_NODES];
    uint32_t caps[SWARM_MAX_NODES];
    uint8_t n = 0;

    ids[n] = self.node_id;
    boots[n] = self.boot_id;
    caps[n] = self_caps;
    ++n;

    if (membership.count > 0) {
        for (uint8_t i = 0; i < membership.count && n < SWARM_MAX_NODES; ++i) {
            NodeId id = membership.members[i];
            if (id == self.node_id) {
                continue;
            }
            const NeighborRecord* rec = neighbor_find_const(neighbors, id);
            if (rec == nullptr || !rec->alive) {
                continue;
            }
            ids[n] = id;
            boots[n] = rec->boot_id;
            caps[n] = rec->capabilities ? rec->capabilities : CAP_COORD_ELIGIBLE;
            ++n;
        }
    } else {
        for (uint8_t i = 0; i < neighbors.count && n < SWARM_MAX_NODES; ++i) {
            if (!neighbors.rows[i].alive) {
                continue;
            }
            ids[n] = neighbors.rows[i].node_id;
            boots[n] = neighbors.rows[i].boot_id;
            caps[n] = neighbors.rows[i].capabilities
                          ? neighbors.rows[i].capabilities
                          : CAP_COORD_ELIGIBLE;
            ++n;
        }
    }

    return elect_highest_eligible(ids, boots, caps, n);
}

bool should_start_election(const SwarmRuntime& rt, uint32_t now_ms) {
    if (rt.coordinator == 0) {
        return true;
    }
    if (rt.coordinator_is_self) {
        return false;
    }
    if (now_ms - rt.last_coord_seen_ms > SWARM_COORDINATOR_TIMEOUT_MS) {
        return true;
    }
    return false;
}
