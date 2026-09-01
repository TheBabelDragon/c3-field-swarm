#pragma once

#include <stdint.h>

#include "config.h"
#include "swarm/protocol.h"
#include "field/field_state.h"

struct FieldDelta {
    uint32_t source_node;
    uint64_t tick;
    Channel channel;
    float old_value;
    float new_value;
};

struct FieldDeltaList {
    FieldDelta items[SWARM_MAX_DELTAS];
    uint8_t count;
};

inline void field_delta_list_clear(FieldDeltaList& list) {
    list.count = 0;
}

inline bool field_delta_list_push(FieldDeltaList& list, const FieldDelta& d) {
    if (list.count >= SWARM_MAX_DELTAS) {
        return false;
    }
    list.items[list.count++] = d;
    return true;
}

void field_delta_sort(FieldDeltaList& list);
bool field_delta_validate(const FieldDelta& d, const FieldState& current);
uint8_t field_delta_apply(FieldState& state, const FieldDeltaList& list);
