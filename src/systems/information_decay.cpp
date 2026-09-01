#include "systems/information_decay.h"

#include <math.h>

InformationDecaySystem::InformationDecaySystem(float decay_rate) : decay_rate_(decay_rate) {}

const char* InformationDecaySystem::name() const {
    return "information_decay";
}

void InformationDecaySystem::set_rate(float rate) {
    decay_rate_ = rate;
}

float InformationDecaySystem::rate() const {
    return decay_rate_;
}

void InformationDecaySystem::evaluate(const FieldView& view, FieldDeltaList& out) {
    float local = view.local.information;
    if (local == 0.0f) {
        return;
    }
    float next = local * expf(-decay_rate_ * view.dt);
    FieldDelta d;
    d.source_node = view.self_id;
    d.tick = view.tick;
    d.channel = CH_INFORMATION;
    d.old_value = local;
    d.new_value = next;
    field_delta_list_push(out, d);
}
