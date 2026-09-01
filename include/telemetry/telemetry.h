#pragma once

#ifdef ARDUINO

#include "swarm/state.h"
#include "field/field_state.h"
#include "swarm/tick.h"

void telemetry_print(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock);
void telemetry_maybe(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock, uint32_t now_ms);

#endif
