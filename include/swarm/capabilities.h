#pragma once

#include <stdint.h>

enum Capability : uint32_t {
    CAP_NONE      = 0,
    CAP_SENSOR    = 1u << 0,
    CAP_ACTUATOR  = 1u << 1,
    CAP_LED       = 1u << 2,
    CAP_ANALOG    = 1u << 3,
    CAP_I2C       = 1u << 4,
    CAP_COORD_ELIGIBLE = 1u << 5,
};

inline uint32_t default_capabilities() {
    return CAP_COORD_ELIGIBLE;
}

inline bool has_capability(uint32_t caps, Capability cap) {
    return (caps & static_cast<uint32_t>(cap)) != 0;
}
