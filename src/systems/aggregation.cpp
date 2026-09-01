#include "systems/aggregation.h"

AggregationSystem::AggregationSystem() {}

const char* AggregationSystem::name() const {
    return "aggregation";
}

void AggregationSystem::evaluate(const FieldView& view, FieldDeltaList& out) {
    float energy = view.local.energy;
    float signal = view.local.signal;
    float info = view.local.information;
    float next = 0.5f * energy + 0.3f * signal + 0.2f * info;
    if (next == energy) {
        return;
    }
    FieldDelta d;
    d.source_node = view.self_id;
    d.tick = view.tick;
    d.channel = CH_ENERGY;
    d.old_value = energy;
    d.new_value = next;
    field_delta_list_push(out, d);
}
