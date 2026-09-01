#pragma once

#include "field/field_state.h"
#include "swarm/state.h"

struct FieldView {
    NodeId self_id;
    uint64_t tick;
    float dt;
    FieldState local;
    NeighborField neighbors[SWARM_MAX_NODES];
    uint8_t neighbor_count;
};

void field_view_freeze(FieldView& view,
                       const FieldStore& store,
                       NodeId self_id,
                       uint64_t tick,
                       float dt);
