#pragma once
#include <stdint.h>
#include <stddef.h>
#include "phy_config.h"

struct BylightHalf {
    uint8_t level;
    uint16_t duration_us;
};

size_t bylight_manchester_encode_byte(uint8_t value, BylightHalf* out, size_t cap, BylightPolarity polarity);
size_t bylight_manchester_encode(const uint8_t* data, size_t length, BylightHalf* out, size_t cap, BylightPolarity polarity);
size_t bylight_tx_encode_frame(uint8_t sequence, const uint8_t* payload, uint8_t payload_len,
                               BylightHalf* out, size_t cap, BylightPolarity polarity);
bool bylight_tx_pulses_bounded(const BylightHalf* halves, size_t count);
void bylight_tx_init();
bool bylight_tx_busy();
void bylight_tx_abort();
bool bylight_tx_send_frame(uint8_t sequence, const uint8_t* payload, uint8_t payload_len);

enum BylightDiagPattern : uint8_t {
    BYLIGHT_DIAG_00 = 0, BYLIGHT_DIAG_FF, BYLIGHT_DIAG_55, BYLIGHT_DIAG_AA,
    BYLIGHT_DIAG_D3, BYLIGHT_DIAG_FRAME,
};
size_t bylight_tx_diag_pattern(BylightDiagPattern p, BylightHalf* out, size_t cap);
bool bylight_tx_idle_is_off();
