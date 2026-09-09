#include <unity.h>
#include <string.h>

#include "swarm/protocol.h"
#include "swarm/capabilities.h"
#include "swarm/state.h"
#include "transport/packet_codec.h"

static PacketHeader sample_header(uint8_t type) {
    PacketHeader h = {};
    h.version = kPacketVersion;
    h.type = type;
    h.length = 0;
    h.sender_id = 3;
    h.boot_id = 99;
    h.sequence = 7;
    h.tick = 42;
    return h;
}

static void assert_header_roundtrip(const PacketHeader& expected, const PacketHeader& got) {
    TEST_ASSERT_EQUAL_UINT32(expected.sender_id, got.sender_id);
    TEST_ASSERT_EQUAL_UINT32(expected.boot_id, got.boot_id);
    TEST_ASSERT_EQUAL_UINT64(expected.sequence, got.sequence);
    TEST_ASSERT_EQUAL_UINT64(expected.tick, got.tick);
}

void test_packet_type_known() {
    TEST_ASSERT_TRUE(packet_type_known(PKT_HELLO));
    TEST_ASSERT_TRUE(packet_type_known(PKT_STATE_REQUEST));
    TEST_ASSERT_FALSE(packet_type_known(0));
    TEST_ASSERT_FALSE(packet_type_known(255));
}

void test_hello_roundtrip() {
    HelloPayload p = {};
    p.hardware_id = 0xAABBCCDDEEFF0011ull;
    p.node_id = 4;
    p.boot_id = 123;
    p.protocol_version = SWARM_PROTOCOL_VERSION;
    p.firmware_version = SWARM_FIRMWARE_VERSION;
    p.reserved = 0;
    p.capabilities = CAP_COORD_ELIGIBLE;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    PacketHeader header_in = sample_header(PKT_HELLO);
    TEST_ASSERT_TRUE(encode_hello(buf, sizeof(buf), n, header_in, p));
    TEST_ASSERT_TRUE(n > kPacketHeaderSize);
    PacketHeader h;
    TEST_ASSERT_TRUE(validate_packet(buf, n, h));
    TEST_ASSERT_EQUAL_UINT8(PKT_HELLO, h.type);
    HelloPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_hello(buf, n, dh, q));
    TEST_ASSERT_EQUAL_UINT64(p.hardware_id, q.hardware_id);
    TEST_ASSERT_EQUAL_UINT32(p.node_id, q.node_id);
    TEST_ASSERT_EQUAL_UINT32(p.boot_id, q.boot_id);
    TEST_ASSERT_EQUAL_UINT32(p.capabilities, q.capabilities);
    assert_header_roundtrip(header_in, dh);
}

void test_hello_roundtrip_header_fields() {
    HelloPayload p = {};
    p.hardware_id = 0xAABBCCDDEEFF0011ull;
    p.node_id = 4;
    p.boot_id = 123;
    p.protocol_version = SWARM_PROTOCOL_VERSION;
    p.firmware_version = SWARM_FIRMWARE_VERSION;
    p.capabilities = CAP_COORD_ELIGIBLE;

    PacketHeader header_in = sample_header(PKT_HELLO);

    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    TEST_ASSERT_TRUE(encode_hello(buf, sizeof(buf), n, header_in, p));

    HelloPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_hello(buf, n, dh, q));

    TEST_ASSERT_EQUAL_UINT32(header_in.sender_id, dh.sender_id);
    TEST_ASSERT_EQUAL_UINT32(header_in.sequence, dh.sequence);
    TEST_ASSERT_EQUAL_UINT32(header_in.boot_id, dh.boot_id);
    TEST_ASSERT_EQUAL_UINT64(header_in.tick, dh.tick);
}

void test_heartbeat_roundtrip() {
    HeartbeatPayload p = {};
    p.node_id = 2;
    p.boot_id = 8;
    p.tick = 1001;
    p.uptime_ms = 5000;
    p.free_heap = 64000;
    p.local_state_version = 12;
    p.coordinator_id = 6;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    PacketHeader header_in = sample_header(PKT_HEARTBEAT);
    TEST_ASSERT_TRUE(encode_heartbeat(buf, sizeof(buf), n, header_in, p));
    HeartbeatPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_heartbeat(buf, n, dh, q));
    TEST_ASSERT_EQUAL_UINT32(6, q.coordinator_id);
    TEST_ASSERT_EQUAL_UINT64(1001, q.tick);
    assert_header_roundtrip(header_in, dh);
}

void test_membership_roundtrip() {
    MembershipPayload p = {};
    p.epoch = 17;
    p.coordinator = 5;
    p.count = 3;
    p.members[0] = 1;
    p.members[1] = 3;
    p.members[2] = 5;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    PacketHeader header_in = sample_header(PKT_MEMBERSHIP);
    TEST_ASSERT_TRUE(encode_membership(buf, sizeof(buf), n, header_in, p));
    MembershipPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_membership(buf, n, dh, q));
    TEST_ASSERT_EQUAL_UINT64(17, q.epoch);
    TEST_ASSERT_EQUAL_UINT32(5, q.coordinator);
    TEST_ASSERT_EQUAL_UINT8(3, q.count);
    TEST_ASSERT_EQUAL_UINT32(3, q.members[1]);
    assert_header_roundtrip(header_in, dh);
}

void test_membership_rejects_stale_epoch() {
    MembershipPayload p_new = {};
    p_new.epoch = 20;
    p_new.coordinator = 5;
    p_new.count = 2;
    p_new.members[0] = 1;
    p_new.members[1] = 5;

    MembershipPayload p_old = {};
    p_old.epoch = 17;
    p_old.coordinator = 3;
    p_old.count = 1;
    p_old.members[0] = 3;

    uint64_t current_epoch = p_new.epoch;

    TEST_ASSERT_FALSE(membership_epoch_is_newer(p_old.epoch, current_epoch));
    TEST_ASSERT_TRUE(membership_epoch_is_newer(p_new.epoch + 1, current_epoch));
}

void test_membership_rejects_count_overflow() {
    MembershipPayload p = {};
    p.epoch = 1;
    p.coordinator = 1;
    p.count = SWARM_MAX_MEMBERS + 1;

    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;

    bool encoded = encode_membership(buf, sizeof(buf), n, sample_header(PKT_MEMBERSHIP), p);
    if (encoded) {
        MembershipPayload q = {};
        PacketHeader dh;
        TEST_ASSERT_FALSE(decode_membership(buf, n, dh, q));
    } else {
        TEST_ASSERT_FALSE(encoded);
    }
}

void test_encode_rejects_undersized_buffer() {
    uint8_t tiny_buf[4];
    size_t n = 0;

    HelloPayload hp = {};
    hp.node_id = 1;
    TEST_ASSERT_FALSE(encode_hello(tiny_buf, sizeof(tiny_buf), n, sample_header(PKT_HELLO), hp));

    HeartbeatPayload bp = {};
    bp.node_id = 1;
    TEST_ASSERT_FALSE(encode_heartbeat(tiny_buf, sizeof(tiny_buf), n, sample_header(PKT_HEARTBEAT), bp));

    MembershipPayload mp = {};
    mp.count = 1;
    TEST_ASSERT_FALSE(encode_membership(tiny_buf, sizeof(tiny_buf), n, sample_header(PKT_MEMBERSHIP), mp));

    FieldTickPayload ftp = {};
    TEST_ASSERT_FALSE(encode_field_tick(tiny_buf, sizeof(tiny_buf), n, sample_header(PKT_FIELD_TICK), ftp));

    FieldState s;
    field_state_zero(s);
    TEST_ASSERT_FALSE(encode_state_delta(tiny_buf, sizeof(tiny_buf), n, sample_header(PKT_STATE_DELTA), s, 1));
}

void test_field_tick_and_delta_roundtrip() {
    FieldTickPayload t = {};
    t.tick = 88;
    t.dt = 0.5f;
    t.coordinator_id = 6;
    t.membership_epoch = 4;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    PacketHeader header_in = sample_header(PKT_FIELD_TICK);
    TEST_ASSERT_TRUE(encode_field_tick(buf, sizeof(buf), n, header_in, t));
    FieldTickPayload t2 = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_field_tick(buf, n, dh, t2));
    TEST_ASSERT_EQUAL_UINT64(88, t2.tick);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, t2.dt);
    assert_header_roundtrip(header_in, dh);

    FieldState s;
    field_state_zero(s);
    s.information = 0.25f;
    s.signal = 0.5f;
    n = 0;
    PacketHeader delta_in = sample_header(PKT_STATE_DELTA);
    TEST_ASSERT_TRUE(encode_state_delta(buf, sizeof(buf), n, delta_in, s, 9));
    FieldState s2;
    field_state_zero(s2);
    uint32_t ver = 0;
    TEST_ASSERT_TRUE(decode_state_delta(buf, n, dh, s2, ver));
    TEST_ASSERT_EQUAL_UINT32(9, ver);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, s2.information);
    assert_header_roundtrip(delta_in, dh);
}

void test_validate_rejects_bad_version_and_truncation() {
    HelloPayload p = {};
    p.node_id = 1;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    TEST_ASSERT_TRUE(encode_hello(buf, sizeof(buf), n, sample_header(PKT_HELLO), p));
    buf[0] = 99;
    PacketHeader h;
    TEST_ASSERT_FALSE(validate_packet(buf, n, h));
    buf[0] = kPacketVersion;
    TEST_ASSERT_FALSE(validate_packet(buf, 10, h));
    TEST_ASSERT_FALSE(validate_packet(nullptr, n, h));
}

void test_validate_rejects_mid_payload_truncation() {
    HelloPayload p = {};
    p.node_id = 1;
    p.hardware_id = 0x1122334455667788ull;

    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    TEST_ASSERT_TRUE(encode_hello(buf, sizeof(buf), n, sample_header(PKT_HELLO), p));

    PacketHeader h;
    size_t mid_truncated = kPacketHeaderSize + 3;
    TEST_ASSERT_TRUE(mid_truncated < n);
    TEST_ASSERT_FALSE(validate_packet(buf, mid_truncated, h));
}

void test_header_is_28_bytes() {
    TEST_ASSERT_TRUE(kPacketHeaderSize == 28);
    uint8_t buf[64];
    size_t off = 0;
    PacketHeader h = sample_header(PKT_GOODBYE);
    h.length = 28;
    TEST_ASSERT_TRUE(encode_header(buf, sizeof(buf), off, h));
    TEST_ASSERT_EQUAL_UINT32(28, (uint32_t)off);
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_packet_type_known);
    RUN_TEST(test_hello_roundtrip);
    RUN_TEST(test_hello_roundtrip_header_fields);
    RUN_TEST(test_heartbeat_roundtrip);
    RUN_TEST(test_membership_roundtrip);
    RUN_TEST(test_membership_rejects_stale_epoch);
    RUN_TEST(test_membership_rejects_count_overflow);
    RUN_TEST(test_encode_rejects_undersized_buffer);
    RUN_TEST(test_field_tick_and_delta_roundtrip);
    RUN_TEST(test_validate_rejects_bad_version_and_truncation);
    RUN_TEST(test_validate_rejects_mid_payload_truncation);
    RUN_TEST(test_header_is_28_bytes);
    return UNITY_END();
}
