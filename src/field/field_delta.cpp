#include "field/field_delta.h"

#include <math.h>

static int delta_cmp(const FieldDelta& a, const FieldDelta& b) {
    if (a.tick < b.tick) return -1;
    if (a.tick > b.tick) return 1;
    if (a.source_node < b.source_node) return -1;
    if (a.source_node > b.source_node) return 1;
    if (static_cast<uint8_t>(a.channel) < static_cast<uint8_t>(b.channel)) return -1;
    if (static_cast<uint8_t>(a.channel) > static_cast<uint8_t>(b.channel)) return 1;
    return 0;
}

void field_delta_sort(FieldDeltaList& list) {
    for (uint8_t i = 0; i < list.count; ++i) {
        for (uint8_t j = static_cast<uint8_t>(i + 1); j < list.count; ++j) {
            if (delta_cmp(list.items[j], list.items[i]) < 0) {
                FieldDelta tmp = list.items[i];
                list.items[i] = list.items[j];
                list.items[j] = tmp;
            }
        }
    }
}

bool field_delta_validate(const FieldDelta& d, const FieldState& current) {
    if (static_cast<uint8_t>(d.channel) >= CH_COUNT) {
        return false;
    }
    if (!isfinite(d.new_value) || !isfinite(d.old_value)) {
        return false;
    }
    (void)current;
    return true;
}

uint8_t field_delta_apply(FieldState& state, const FieldDeltaList& list) {
    uint8_t applied = 0;
    for (uint8_t i = 0; i < list.count; ++i) {
        const FieldDelta& d = list.items[i];
        if (!field_delta_validate(d, state)) {
            continue;
        }
        field_channel_set(state, d.channel, d.new_value);
        ++applied;
    }
    return applied;
}
