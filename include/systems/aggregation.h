#pragma once

#include "field/field_system.h"

class AggregationSystem : public FieldSystem {
public:
    AggregationSystem();
    const char* name() const override;
    void evaluate(const FieldView& view, FieldDeltaList& out) override;
};
