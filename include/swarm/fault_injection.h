#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef SWARM_FAULT_INJECTION

struct FaultConfig {
    uint8_t drop_percent;
    uint8_t duplicate_percent;
    uint8_t reorder_percent;
    uint8_t corrupt_percent;
};

bool fault_should_drop(uint64_t sequence);
bool fault_should_duplicate(uint64_t sequence);
bool fault_should_reorder(uint64_t sequence);

#else

inline bool fault_should_drop(uint64_t) { return false; }
inline bool fault_should_duplicate(uint64_t) { return false; }
inline bool fault_should_reorder(uint64_t) { return false; }

#endif
