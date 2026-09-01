#pragma once

#include "swarm/state.h"
#include "swarm/capabilities.h"

struct ElectionResult {
    NodeId winner;
    BootId winner_boot;
    bool valid;
};

ElectionResult elect_highest_eligible(const NodeId* ids,
                                      const BootId* boots,
                                      const uint32_t* capabilities,
                                      uint8_t count);

ElectionResult elect_from_membership(const Membership& membership,
                                     const NeighborTable& neighbors,
                                     const NodeIdentity& self,
                                     uint32_t self_caps);

bool should_start_election(const SwarmRuntime& rt, uint32_t now_ms);
