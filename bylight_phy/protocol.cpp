#include "protocol.h"

uint16_t bylight_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = BYLIGHT_CRC_INIT;
    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000u) {
                crc = static_cast<uint16_t>((crc << 1) ^ BYLIGHT_CRC_POLY);
            } else {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }
    return crc;
}

size_t bylight_frame_build(uint8_t* out, size_t cap, uint8_t sequence,
                           const uint8_t* payload, uint8_t payload_len) {
    if (out == nullptr || payload_len > BYLIGHT_MAX_PAYLOAD) return 0;
    const size_t need = bylight_frame_overhead() + payload_len;
    if (cap < need) return 0;
    size_t n = 0;
    for (uint8_t i = 0; i < BYLIGHT_PREAMBLE_COUNT; ++i) out[n++] = BYLIGHT_PREAMBLE_BYTE;
    out[n++] = BYLIGHT_SYNC;
    const size_t crc_start = n;
    out[n++] = BYLIGHT_VERSION;
    out[n++] = payload_len;
    out[n++] = sequence;
    for (uint8_t i = 0; i < payload_len; ++i) out[n++] = payload ? payload[i] : 0;
    const uint16_t crc = bylight_crc16(out + crc_start, 3 + payload_len);
    out[n++] = static_cast<uint8_t>(crc & 0xFF);
    out[n++] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    out[n++] = BYLIGHT_END;
    return n;
}

static bool has_preamble_at(const uint8_t* data, size_t length, size_t off) {
    if (off + BYLIGHT_PREAMBLE_COUNT > length) return false;
    for (uint8_t i = 0; i < BYLIGHT_PREAMBLE_COUNT; ++i) {
        if (data[off + i] != BYLIGHT_PREAMBLE_BYTE) return false;
    }
    return true;
}

BylightParseResult bylight_frame_parse(const uint8_t* data, size_t length,
                                       BylightFrame& frame, size_t* consumed) {
    if (consumed) *consumed = 0;
    if (data == nullptr || length < bylight_frame_overhead()) return BYLIGHT_PARSE_TOO_SHORT;
    size_t off = 0;
    while (off < length && !has_preamble_at(data, length, off)) ++off;
    if (!has_preamble_at(data, length, off)) return BYLIGHT_PARSE_NO_PREAMBLE;
    off += BYLIGHT_PREAMBLE_COUNT;
    if (off >= length) return BYLIGHT_PARSE_TRUNCATED;
    if (data[off++] != BYLIGHT_SYNC) return BYLIGHT_PARSE_BAD_SYNC;
    if (off + 3 > length) return BYLIGHT_PARSE_TRUNCATED;
    const size_t crc_start = off;
    const uint8_t version = data[off++];
    const uint8_t plen = data[off++];
    const uint8_t seq = data[off++];
    if (version != BYLIGHT_VERSION) return BYLIGHT_PARSE_BAD_VERSION;
    if (plen > BYLIGHT_MAX_PAYLOAD) return BYLIGHT_PARSE_BAD_LENGTH;
    if (off + plen + 3 > length) return BYLIGHT_PARSE_TRUNCATED;
    frame.version = version;
    frame.length = plen;
    frame.sequence = seq;
    for (uint8_t i = 0; i < plen; ++i) frame.payload[i] = data[off++];
    const uint16_t crc_got = static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
    off += 2;
    if (crc_got != bylight_crc16(data + crc_start, 3 + plen)) return BYLIGHT_PARSE_BAD_CRC;
    if (data[off++] != BYLIGHT_END) return BYLIGHT_PARSE_BAD_END;
    if (consumed) *consumed = off;
    return BYLIGHT_PARSE_OK;
}

const char* bylight_parse_name(BylightParseResult r) {
    switch (r) {
        case BYLIGHT_PARSE_OK: return "OK";
        case BYLIGHT_PARSE_TOO_SHORT: return "TOO_SHORT";
        case BYLIGHT_PARSE_NO_PREAMBLE: return "NO_PREAMBLE";
        case BYLIGHT_PARSE_BAD_SYNC: return "BAD_SYNC";
        case BYLIGHT_PARSE_BAD_VERSION: return "BAD_VERSION";
        case BYLIGHT_PARSE_BAD_LENGTH: return "BAD_LENGTH";
        case BYLIGHT_PARSE_TRUNCATED: return "TRUNCATED";
        case BYLIGHT_PARSE_BAD_CRC: return "BAD_CRC";
        case BYLIGHT_PARSE_BAD_END: return "BAD_END";
        default: return "?";
    }
}
