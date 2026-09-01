#pragma once

#include <stdint.h>

#include "config.h"

using NodeId = uint32_t;
using BootId = uint32_t;
using HardwareId = uint64_t;

struct NodeIdentity {
    HardwareId hardware_id;
    NodeId node_id;
    BootId boot_id;
};

inline bool identity_same_node(const NodeIdentity& a, const NodeIdentity& b) {
    return a.node_id == b.node_id;
}

inline bool identity_same_boot(const NodeIdentity& a, const NodeIdentity& b) {
    return a.node_id == b.node_id && a.boot_id == b.boot_id;
}

#ifdef ARDUINO
bool node_identity_begin(NodeIdentity& out);
void node_identity_force_id(NodeIdentity& id, NodeId node_id);
#endif
