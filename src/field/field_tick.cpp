#include "field/field_view.h"

void field_view_freeze(FieldView& view,
                       const FieldStore& store,
                       NodeId self_id,
                       uint64_t tick,
                       float dt) {
    view.self_id = self_id;
    view.tick = tick;
    view.dt = dt;
    view.local = store.local;
    view.neighbor_count = store.neighbor_count;
    for (uint8_t i = 0; i < store.neighbor_count; ++i) {
        view.neighbors[i] = store.neighbors[i];
    }
    for (uint8_t i = store.neighbor_count; i < SWARM_MAX_NODES; ++i) {
        view.neighbors[i].node_id = 0;
        view.neighbors[i].valid = false;
        field_state_zero(view.neighbors[i].state);
        view.neighbors[i].version = 0;
    }
}
