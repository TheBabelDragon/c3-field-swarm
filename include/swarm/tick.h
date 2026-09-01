#pragma once

#include <stdint.h>

#include "config.h"

struct TickClock {
    uint64_t tick;
    float dt;
    uint32_t last_advance_ms;
    bool frozen;
};

inline void tick_clock_init(TickClock& clock, float dt) {
    clock.tick = 0;
    clock.dt = dt;
    clock.last_advance_ms = 0;
    clock.frozen = false;
}

inline void tick_clock_set(TickClock& clock, uint64_t tick, float dt) {
    if (tick >= clock.tick) {
        clock.tick = tick;
        clock.dt = dt;
    }
}

inline uint64_t tick_clock_advance_local(TickClock& clock) {
    if (!clock.frozen) {
        clock.tick += 1;
    }
    return clock.tick;
}
