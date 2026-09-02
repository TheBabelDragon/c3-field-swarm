#include "transport/packet_codec.h"

#include <string.h>

bool codec_write_u8(uint8_t* buf, size_t cap, size_t& off, uint8_t v) {
    if (buf == nullptr || off + 1 > cap) {
        return false;
    }
    buf[off++] = v;
    return true;
}

bool codec_write_u16(uint8_t* buf, size_t cap, size_t& off, uint16_t v) {
    if (buf == nullptr || off + 2 > cap) {
        return false;
    }
    buf[off++] = static_cast<uint8_t>(v);
    buf[off++] = static_cast<uint8_t>(v >> 8);
    return true;
}

bool codec_write_u32(uint8_t* buf, size_t cap, size_t& off, uint32_t v) {
    if (buf == nullptr || off + 4 > cap) {
        return false;
    }
    buf[off++] = static_cast<uint8_t>(v);
    buf[off++] = static_cast<uint8_t>(v >> 8);
    buf[off++] = static_cast<uint8_t>(v >> 16);
    buf[off++] = static_cast<uint8_t>(v >> 24);
    return true;
}

bool codec_write_u64(uint8_t* buf, size_t cap, size_t& off, uint64_t v) {
    if (!codec_write_u32(buf, cap, off, static_cast<uint32_t>(v))) {
        return false;
    }
    return codec_write_u32(buf, cap, off, static_cast<uint32_t>(v >> 32));
}

bool codec_write_f32(uint8_t* buf, size_t cap, size_t& off, float v) {
    uint32_t bits = 0;
    memcpy(&bits, &v, sizeof(bits));
    return codec_write_u32(buf, cap, off, bits);
}

bool codec_write_bytes(uint8_t* buf, size_t cap, size_t& off, const void* src, size_t n) {
    if (src == nullptr || buf == nullptr || off + n > cap) {
        return false;
    }
    memcpy(buf + off, src, n);
    off += n;
    return true;
}

bool codec_read_u8(const uint8_t* buf, size_t len, size_t& off, uint8_t& v) {
    if (buf == nullptr || off + 1 > len) {
        return false;
    }
    v = buf[off++];
    return true;
}

bool codec_read_u16(const uint8_t* buf, size_t len, size_t& off, uint16_t& v) {
    if (buf == nullptr || off + 2 > len) {
        return false;
    }
    v = static_cast<uint16_t>(buf[off] | (static_cast<uint16_t>(buf[off + 1]) << 8));
    off += 2;
    return true;
}

bool codec_read_u32(const uint8_t* buf, size_t len, size_t& off, uint32_t& v) {
    if (buf == nullptr || off + 4 > len) {
        return false;
    }
    v = static_cast<uint32_t>(buf[off])
        | (static_cast<uint32_t>(buf[off + 1]) << 8)
        | (static_cast<uint32_t>(buf[off + 2]) << 16)
        | (static_cast<uint32_t>(buf[off + 3]) << 24);
    off += 4;
    return true;
}

bool codec_read_u64(const uint8_t* buf, size_t len, size_t& off, uint64_t& v) {
    uint32_t lo = 0;
    uint32_t hi = 0;
    if (!codec_read_u32(buf, len, off, lo) || !codec_read_u32(buf, len, off, hi)) {
        return false;
    }
    v = static_cast<uint64_t>(lo) | (static_cast<uint64_t>(hi) << 32);
    return true;
}

bool codec_read_f32(const uint8_t* buf, size_t len, size_t& off, float& v) {
    uint32_t bits = 0;
    if (!codec_read_u32(buf, len, off, bits)) {
        return false;
    }
    memcpy(&v, &bits, sizeof(v));
    return true;
}

bool encode_header(uint8_t* buf, size_t cap, size_t& off, const PacketHeader& h) {
    if (!codec_write_u8(buf, cap, off, h.version)) return false;
    if (!codec_write_u8(buf, cap, off, h.type)) return false;
    if (!codec_write_u16(buf, cap, off, h.length)) return false;
    if (!codec_write_u32(buf, cap, off, h.sender_id)) return false;
    if (!codec_write_u32(buf, cap, off, h.boot_id)) return false;
    if (!codec_write_u64(buf, cap, off, h.sequence)) return false;
    if (!codec_write_u64(buf, cap, off, h.tick)) return false;
    return true;
}

bool decode_header(const uint8_t* buf, size_t len, size_t& off, PacketHeader& h) {
    if (!codec_read_u8(buf, len, off, h.version)) return false;
    if (!codec_read_u8(buf, len, off, h.type)) return false;
    if (!codec_read_u16(buf, len, off, h.length)) return false;
    if (!codec_read_u32(buf, len, off, h.sender_id)) return false;
    if (!codec_read_u32(buf, len, off, h.boot_id)) return false;
    if (!codec_read_u64(buf, len, off, h.sequence)) return false;
    if (!codec_read_u64(buf, len, off, h.tick)) return false;
    return true;
}

static bool patch_length(uint8_t* buf, size_t cap, size_t total) {
    if (total > 0xFFFFu || total > cap) {
        return false;
    }
    size_t off = 2;
    return codec_write_u16(buf, cap, off, static_cast<uint16_t>(total));
}

static bool write_header_placeholder(uint8_t* buf, size_t cap, PacketHeader h) {
    size_t off = 0;
    h.length = 0;
    return encode_header(buf, cap, off, h);
}

bool encode_hello(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const HelloPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u64(buf, cap, off, p.hardware_id)) return false;
    if (!codec_write_u32(buf, cap, off, p.node_id)) return false;
    if (!codec_write_u32(buf, cap, off, p.boot_id)) return false;
    if (!codec_write_u8(buf, cap, off, p.protocol_version)) return false;
    if (!codec_write_u8(buf, cap, off, p.firmware_version)) return false;
    if (!codec_write_u16(buf, cap, off, p.reserved)) return false;
    if (!codec_write_u32(buf, cap, off, p.capabilities)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_hello(const uint8_t* buf, size_t len, PacketHeader& h, HelloPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u64(buf, len, off, p.hardware_id)) return false;
    if (!codec_read_u32(buf, len, off, p.node_id)) return false;
    if (!codec_read_u32(buf, len, off, p.boot_id)) return false;
    if (!codec_read_u8(buf, len, off, p.protocol_version)) return false;
    if (!codec_read_u8(buf, len, off, p.firmware_version)) return false;
    if (!codec_read_u16(buf, len, off, p.reserved)) return false;
    if (!codec_read_u32(buf, len, off, p.capabilities)) return false;
    return true;
}

bool encode_heartbeat(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const HeartbeatPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u32(buf, cap, off, p.node_id)) return false;
    if (!codec_write_u32(buf, cap, off, p.boot_id)) return false;
    if (!codec_write_u64(buf, cap, off, p.tick)) return false;
    if (!codec_write_u32(buf, cap, off, p.uptime_ms)) return false;
    if (!codec_write_u32(buf, cap, off, p.free_heap)) return false;
    if (!codec_write_u32(buf, cap, off, p.local_state_version)) return false;
    if (!codec_write_u32(buf, cap, off, p.coordinator_id)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_heartbeat(const uint8_t* buf, size_t len, PacketHeader& h, HeartbeatPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u32(buf, len, off, p.node_id)) return false;
    if (!codec_read_u32(buf, len, off, p.boot_id)) return false;
    if (!codec_read_u64(buf, len, off, p.tick)) return false;
    if (!codec_read_u32(buf, len, off, p.uptime_ms)) return false;
    if (!codec_read_u32(buf, len, off, p.free_heap)) return false;
    if (!codec_read_u32(buf, len, off, p.local_state_version)) return false;
    if (!codec_read_u32(buf, len, off, p.coordinator_id)) return false;
    return true;
}

bool encode_membership(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const MembershipPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u64(buf, cap, off, p.epoch)) return false;
    if (!codec_write_u32(buf, cap, off, p.coordinator)) return false;
    if (!codec_write_u8(buf, cap, off, p.count)) return false;
    if (!codec_write_bytes(buf, cap, off, p.reserved, 3)) return false;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        if (!codec_write_u32(buf, cap, off, p.members[i])) return false;
    }
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_membership(const uint8_t* buf, size_t len, PacketHeader& h, MembershipPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u64(buf, len, off, p.epoch)) return false;
    if (!codec_read_u32(buf, len, off, p.coordinator)) return false;
    if (!codec_read_u8(buf, len, off, p.count)) return false;
    uint8_t r0 = 0, r1 = 0, r2 = 0;
    if (!codec_read_u8(buf, len, off, r0) || !codec_read_u8(buf, len, off, r1) || !codec_read_u8(buf, len, off, r2)) {
        return false;
    }
    p.reserved[0] = r0;
    p.reserved[1] = r1;
    p.reserved[2] = r2;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        if (!codec_read_u32(buf, len, off, p.members[i])) return false;
    }
    return true;
}

bool encode_election(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const ElectionPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u64(buf, cap, off, p.epoch)) return false;
    if (!codec_write_u32(buf, cap, off, p.candidate)) return false;
    if (!codec_write_u32(buf, cap, off, p.candidate_boot)) return false;
    if (!codec_write_u8(buf, cap, off, p.term)) return false;
    if (!codec_write_bytes(buf, cap, off, p.reserved, 3)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_election(const uint8_t* buf, size_t len, PacketHeader& h, ElectionPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u64(buf, len, off, p.epoch)) return false;
    if (!codec_read_u32(buf, len, off, p.candidate)) return false;
    if (!codec_read_u32(buf, len, off, p.candidate_boot)) return false;
    if (!codec_read_u8(buf, len, off, p.term)) return false;
    uint8_t r0 = 0, r1 = 0, r2 = 0;
    if (!codec_read_u8(buf, len, off, r0) || !codec_read_u8(buf, len, off, r1) || !codec_read_u8(buf, len, off, r2)) {
        return false;
    }
    p.reserved[0] = r0;
    p.reserved[1] = r1;
    p.reserved[2] = r2;
    return true;
}

bool encode_digest(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const StateDigestPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u32(buf, cap, off, p.state_version)) return false;
    if (!codec_write_u32(buf, cap, off, p.coordinator_id)) return false;
    if (!codec_write_u64(buf, cap, off, p.tick)) return false;
    if (!codec_write_u32(buf, cap, off, p.checksum)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_digest(const uint8_t* buf, size_t len, PacketHeader& h, StateDigestPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u32(buf, len, off, p.state_version)) return false;
    if (!codec_read_u32(buf, len, off, p.coordinator_id)) return false;
    if (!codec_read_u64(buf, len, off, p.tick)) return false;
    if (!codec_read_u32(buf, len, off, p.checksum)) return false;
    return true;
}

bool encode_field_tick(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const FieldTickPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u64(buf, cap, off, p.tick)) return false;
    if (!codec_write_f32(buf, cap, off, p.dt)) return false;
    if (!codec_write_u32(buf, cap, off, p.coordinator_id)) return false;
    if (!codec_write_u64(buf, cap, off, p.membership_epoch)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_field_tick(const uint8_t* buf, size_t len, PacketHeader& h, FieldTickPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u64(buf, len, off, p.tick)) return false;
    if (!codec_read_f32(buf, len, off, p.dt)) return false;
    if (!codec_read_u32(buf, len, off, p.coordinator_id)) return false;
    if (!codec_read_u64(buf, len, off, p.membership_epoch)) return false;
    return true;
}

bool encode_state_delta(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h,
                        const FieldState& local, uint32_t version) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u32(buf, cap, off, version)) return false;
    if (!codec_write_f32(buf, cap, off, local.temperature)) return false;
    if (!codec_write_f32(buf, cap, off, local.information)) return false;
    if (!codec_write_f32(buf, cap, off, local.energy)) return false;
    if (!codec_write_f32(buf, cap, off, local.signal)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_state_delta(const uint8_t* buf, size_t len, PacketHeader& h,
                        FieldState& local, uint32_t& version) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u32(buf, len, off, version)) return false;
    if (!codec_read_f32(buf, len, off, local.temperature)) return false;
    if (!codec_read_f32(buf, len, off, local.information)) return false;
    if (!codec_read_f32(buf, len, off, local.energy)) return false;
    if (!codec_read_f32(buf, len, off, local.signal)) return false;
    return true;
}

bool encode_command(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const CommandPayload& p) {
    if (!write_header_placeholder(buf, cap, h)) return false;
    size_t off = kPacketHeaderSize;
    if (!codec_write_u32(buf, cap, off, p.command_id)) return false;
    if (!codec_write_u32(buf, cap, off, p.target_node)) return false;
    if (!codec_write_u8(buf, cap, off, p.opcode)) return false;
    if (!codec_write_bytes(buf, cap, off, p.reserved, 3)) return false;
    if (!codec_write_f32(buf, cap, off, p.arg0)) return false;
    if (!codec_write_f32(buf, cap, off, p.arg1)) return false;
    if (!patch_length(buf, cap, off)) return false;
    n = off;
    return true;
}

bool decode_command(const uint8_t* buf, size_t len, PacketHeader& h, CommandPayload& p) {
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) return false;
    if (!codec_read_u32(buf, len, off, p.command_id)) return false;
    if (!codec_read_u32(buf, len, off, p.target_node)) return false;
    if (!codec_read_u8(buf, len, off, p.opcode)) return false;
    uint8_t r0 = 0, r1 = 0, r2 = 0;
    if (!codec_read_u8(buf, len, off, r0) || !codec_read_u8(buf, len, off, r1) || !codec_read_u8(buf, len, off, r2)) {
        return false;
    }
    p.reserved[0] = r0;
    p.reserved[1] = r1;
    p.reserved[2] = r2;
    if (!codec_read_f32(buf, len, off, p.arg0)) return false;
    if (!codec_read_f32(buf, len, off, p.arg1)) return false;
    return true;
}

bool encode_goodbye(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h) {
    PacketHeader hh = h;
    hh.length = static_cast<uint16_t>(kPacketHeaderSize);
    size_t off = 0;
    if (!encode_header(buf, cap, off, hh)) return false;
    n = off;
    return true;
}

bool packet_type_known(uint8_t type) {
    return type >= PKT_HELLO && type <= PKT_STATE_REQUEST;
}

const char* packet_type_name(uint8_t type) {
    switch (type) {
        case PKT_HELLO: return "HELLO";
        case PKT_HELLO_ACK: return "HELLO_ACK";
        case PKT_HEARTBEAT: return "HEARTBEAT";
        case PKT_MEMBERSHIP: return "MEMBERSHIP";
        case PKT_STATE_DIGEST: return "STATE_DIGEST";
        case PKT_STATE_DELTA: return "STATE_DELTA";
        case PKT_FIELD_TICK: return "FIELD_TICK";
        case PKT_COMMAND: return "COMMAND";
        case PKT_COMMAND_ACK: return "COMMAND_ACK";
        case PKT_ELECTION: return "ELECTION";
        case PKT_ELECTION_ACK: return "ELECTION_ACK";
        case PKT_TELEMETRY: return "TELEMETRY";
        case PKT_GOODBYE: return "GOODBYE";
        case PKT_STATE_REQUEST: return "STATE_REQUEST";
        default: return "UNKNOWN";
    }
}

bool validate_packet(const uint8_t* buf, size_t len, PacketHeader& h) {
    if (buf == nullptr || len < kPacketHeaderSize || len > SWARM_MAX_PACKET) {
        return false;
    }
    size_t off = 0;
    if (!decode_header(buf, len, off, h)) {
        return false;
    }
    if (h.version != kPacketVersion) {
        return false;
    }
    if (!packet_type_known(h.type)) {
        return false;
    }
    if (h.length != static_cast<uint16_t>(len)) {
        return false;
    }
    return true;
}
