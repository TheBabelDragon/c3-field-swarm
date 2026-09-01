#include "field/field_system.h"

uint8_t field_run_systems(const FieldView& view,
                          FieldSystem** systems,
                          uint8_t system_count,
                          FieldState& mutable_local,
                          FieldDeltaList& applied) {
    field_delta_list_clear(applied);

    FieldDeltaList proposed;
    field_delta_list_clear(proposed);

    for (uint8_t i = 0; i < system_count; ++i) {
        if (systems[i] == nullptr) {
            continue;
        }
        systems[i]->evaluate(view, proposed);
    }

    field_delta_sort(proposed);
    field_delta_apply(mutable_local, proposed);
    applied = proposed;
    return applied.count;
}
