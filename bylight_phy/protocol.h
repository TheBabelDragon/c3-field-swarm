#pragma once

#include <stdint.h>
#include <stddef.h>

#include "phy_config.h"

struct BylightFrame {
    uint8_t version;
    uint8_t length;
    uint8_t sequence;
    uint8_t payload[BYLIGHT_MAX_PAYLOAD];
};

enum BylightParseResult : uint8_t {
    BYLIGHT_PARSE_OK = 0,
    BYLIGHT_PARSE_TOO_SHORT,
    BYLIGHT_PARSE_NO_PREAMBLE,
    BYLIGHT_PARSE_BAD_SYNC,
    BYLIGHT_PARSE_BAD_VERSION,
    BYLIGHT_PARSE_BAD_LENGTH,
    BYLIGHT_PARSE_TRUNCATED,
    BYLIGHT_PARSE_BAD_CRC,
    BYLIGHT_PARSE_BAD_END,
};

uint16_t bylight_crc16(const uint8_t* data, size_t length);

size_t bylight_frame_build(uint8_t* out,
                           size_t cap,
                           uint8_t sequence,
                           const uint8_t* payload,
                           uint8_t payload_len);

BylightParseResult bylight_frame_parse(const uint8_t* data,
                                       size_t length,
                                       BylightFrame& frame,
                                       size_t* consumed);

const char* bylight_parse_name(BylightParseResult r);
