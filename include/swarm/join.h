#pragma once

#include "config.h"
#include "swarm/node_id.h"

// Pure join policy. Flash-and-join must work with empty NVS and no serial `id`.

inline bool join_in_discovery(uint32_t now_ms, uint32_t until_ms) {
    return until_ms != 0 && now_ms < until_ms;
}

bool join_should_adopt(NodeId self_id,
                       uint8_t self_members,
                       uint64_t self_epoch,
                       NodeId self_coord,
                       bool in_discovery,
                       NodeId foreign_coord,
                       uint8_t foreign_members,
                       uint64_t foreign_epoch);

bool join_should_yield_node_id(HardwareId self_hw, HardwareId other_hw);

NodeId join_next_free_id(const NodeId* taken, uint8_t taken_count);
