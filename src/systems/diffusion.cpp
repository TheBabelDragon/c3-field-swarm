#include "systems/diffusion.h"

DiffusionSystem::DiffusionSystem(float rate) : rate_(rate) {}

const char* DiffusionSystem::name() const {
    return "diffusion";
}

void DiffusionSystem::set_rate(float rate) {
    rate_ = rate;
}

float DiffusionSystem::rate() const {
    return rate_;
}

void DiffusionSystem::evaluate(const FieldView& view, FieldDeltaList& out) {
    if (view.neighbor_count == 0) {
        return;
    }

    const Channel channels[3] = {CH_TEMPERATURE, CH_INFORMATION, CH_SIGNAL};
    for (uint8_t c = 0; c < 3; ++c) {
        Channel ch = channels[c];
        float sum = 0.0f;
        uint8_t n = 0;
        for (uint8_t i = 0; i < view.neighbor_count; ++i) {
            if (!view.neighbors[i].valid) {
                continue;
            }
            if (view.neighbors[i].node_id == view.self_id) {
                continue;
            }
            sum += field_channel_get(view.neighbors[i].state, ch);
            ++n;
        }
        if (n == 0) {
            continue;
        }
        float local = field_channel_get(view.local, ch);
        float avg = sum / static_cast<float>(n);
        float next = local + rate_ * (avg - local);
        if (next == local) {
            continue;
        }
        FieldDelta d;
        d.source_node = view.self_id;
        d.tick = view.tick;
        d.channel = ch;
        d.old_value = local;
        d.new_value = next;
        field_delta_list_push(out, d);
    }
}
