#pragma once

#include "field/field_system.h"

class DiffusionSystem : public FieldSystem {
public:
    explicit DiffusionSystem(float rate = SWARM_DIFFUSION_RATE);
    const char* name() const override;
    void evaluate(const FieldView& view, FieldDeltaList& out) override;
    void set_rate(float rate);
    float rate() const;

private:
    float rate_;
};
