#pragma once

#include "field/field_view.h"
#include "field/field_delta.h"

class FieldSystem {
public:
    virtual ~FieldSystem() {}
    virtual const char* name() const = 0;
    virtual void evaluate(const FieldView& view, FieldDeltaList& out) = 0;
};

uint8_t field_run_systems(const FieldView& view,
                          FieldSystem** systems,
                          uint8_t system_count,
                          FieldState& mutable_local,
                          FieldDeltaList& applied);
