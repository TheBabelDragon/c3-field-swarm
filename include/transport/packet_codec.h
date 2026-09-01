#pragma once

#include <stdint.h>
#include <stddef.h>

#include "swarm/protocol.h"
#include "field/field_state.h"

bool codec_write_u8(uint8_t* buf, size_t cap, size_t& off, uint8_t v);
bool codec_write_u16(uint8_t* buf, size_t cap, size_t& off, uint16_t v);
bool codec_write_u32(uint8_t* buf, size_t cap, size_t& off, uint32_t v);
bool codec_write_u64(uint8_t* buf, size_t cap, size_t& off, uint64_t v);
bool codec_write_f32(uint8_t* buf, size_t cap, size_t& off, float v);
bool codec_write_bytes(uint8_t* buf, size_t cap, size_t& off, const void* src, size_t n);

bool codec_read_u8(const uint8_t* buf, size_t len, size_t& off, uint8_t& v);
bool codec_read_u16(const uint8_t* buf, size_t len, size_t& off, uint16_t& v);
bool codec_read_u32(const uint8_t* buf, size_t len, size_t& off, uint32_t& v);
bool codec_read_u64(const uint8_t* buf, size_t len, size_t& off, uint64_t& v);
bool codec_read_f32(const uint8_t* buf, size_t len, size_t& off, float& v);

bool encode_header(uint8_t* buf, size_t cap, size_t& off, const PacketHeader& h);
bool decode_header(const uint8_t* buf, size_t len, size_t& off, PacketHeader& h);

bool encode_hello(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const HelloPayload& p);
bool decode_hello(const uint8_t* buf, size_t len, PacketHeader& h, HelloPayload& p);

bool encode_heartbeat(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const HeartbeatPayload& p);
bool decode_heartbeat(const uint8_t* buf, size_t len, PacketHeader& h, HeartbeatPayload& p);

bool encode_membership(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const MembershipPayload& p);
bool decode_membership(const uint8_t* buf, size_t len, PacketHeader& h, MembershipPayload& p);

bool encode_election(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const ElectionPayload& p);
bool decode_election(const uint8_t* buf, size_t len, PacketHeader& h, ElectionPayload& p);

bool encode_digest(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const StateDigestPayload& p);
bool decode_digest(const uint8_t* buf, size_t len, PacketHeader& h, StateDigestPayload& p);

bool encode_field_tick(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h, const FieldTickPayload& p);
bool decode_field_tick(const uint8_t* buf, size_t len, PacketHeader& h, FieldTickPayload& p);

bool encode_state_delta(uint8_t* buf, size_t cap, size_t& n, const PacketHeader& h,
                        const FieldState& local, uint32_t version);
bool decode_state_delta(const uint8_t* buf, size_t len, PacketHeader& h,
                        FieldState& local, uint32_t& version);

bool validate_packet(const uint8_t* buf, size_t len, PacketHeader& h);
