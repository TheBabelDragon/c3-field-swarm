#pragma once

#include <stdint.h>
#include <stddef.h>

// bylight_phy v1.1 — single source of truth for protocol and hardware constants

static constexpr uint8_t  BYLIGHT_VERSION          = 1;
static constexpr uint8_t  BYLIGHT_PREAMBLE_BYTE    = 0x55;
static constexpr uint8_t  BYLIGHT_PREAMBLE_COUNT   = 4;
static constexpr uint8_t  BYLIGHT_SYNC             = 0xD3;
static constexpr uint8_t  BYLIGHT_END              = 0x7E;
static constexpr uint8_t  BYLIGHT_MAX_PAYLOAD      = 64;

static constexpr uint16_t BYLIGHT_CRC_POLY         = 0x1021;
static constexpr uint16_t BYLIGHT_CRC_INIT         = 0xFFFF;

enum BylightPolarity : uint8_t {
    BYLIGHT_POLARITY_NORMAL   = 0,
    BYLIGHT_POLARITY_INVERTED = 1,
};

static constexpr uint32_t BYLIGHT_SYMBOL_US              = 1000;
static constexpr uint32_t BYLIGHT_HALF_SYMBOL_US         = BYLIGHT_SYMBOL_US / 2;
static constexpr uint8_t  BYLIGHT_TIMING_TOLERANCE_PERCENT = 30;

static constexpr uint32_t BYLIGHT_MAX_PULSE_US           = 8000;
static constexpr uint8_t  BYLIGHT_MAX_DUTY_PERCENT       = 50;
static constexpr uint32_t BYLIGHT_DUTY_WINDOW_US         = 20000;

static constexpr uint32_t BYLIGHT_R_PD_TOP_OHM           = 330000;
static constexpr uint32_t BYLIGHT_R_PD_BOTTOM_OHM        = 22000;
static constexpr uint32_t BYLIGHT_R_VREF_TOP_OHM         = 100000;
static constexpr uint32_t BYLIGHT_R_VREF_BOTTOM_OHM      = 100000;
static constexpr uint32_t BYLIGHT_R_GATE_OHM             = 100;
static constexpr uint32_t BYLIGHT_R_GATE_PD_OHM          = 100000;
static constexpr uint32_t BYLIGHT_R_LED_OHM              = 33;

#ifndef BYLIGHT_TX_GPIO
#define BYLIGHT_TX_GPIO 4
#endif
#ifndef BYLIGHT_RX_GPIO
#define BYLIGHT_RX_GPIO 5
#endif
#ifndef BYLIGHT_RX_ANA_GPIO
#define BYLIGHT_RX_ANA_GPIO 0
#endif

enum BylightRxPath : uint8_t {
    BYLIGHT_RX_PASSIVE     = 0,
    BYLIGHT_RX_CONDITIONED = 1,
};

static constexpr uint16_t bylight_half_min_us() {
    return static_cast<uint16_t>(
        BYLIGHT_HALF_SYMBOL_US * (100 - BYLIGHT_TIMING_TOLERANCE_PERCENT) / 100);
}

static constexpr uint16_t bylight_half_max_us() {
    return static_cast<uint16_t>(
        BYLIGHT_HALF_SYMBOL_US * (100 + BYLIGHT_TIMING_TOLERANCE_PERCENT) / 100);
}

static constexpr size_t bylight_frame_overhead() {
    return BYLIGHT_PREAMBLE_COUNT + 1 + 1 + 1 + 1 + 2 + 1;
}

static constexpr size_t bylight_max_frame_bytes() {
    return bylight_frame_overhead() + BYLIGHT_MAX_PAYLOAD;
}
