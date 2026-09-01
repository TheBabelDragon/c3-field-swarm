#pragma once

#include "field/field_system.h"

class InformationDecaySystem : public FieldSystem {
public:
    explicit InformationDecaySystem(float decay_rate = SWARM_DECAY_RATE);
    const char* name() const override;
    void evaluate(const FieldView& view, FieldDeltaList& out) override;
    void set_rate(float rate);
    float rate() const;

private:
    float decay_rate_;
};
