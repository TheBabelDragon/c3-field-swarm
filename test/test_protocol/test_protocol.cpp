#include <unity.h>
#include <string.h>

#include "swarm/protocol.h"
#include "swarm/capabilities.h"
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
    TEST_ASSERT_TRUE(encode_hello(buf, sizeof(buf), n, sample_header(PKT_HELLO), p));
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
    TEST_ASSERT_TRUE(encode_heartbeat(buf, sizeof(buf), n, sample_header(PKT_HEARTBEAT), p));
    HeartbeatPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_heartbeat(buf, n, dh, q));
    TEST_ASSERT_EQUAL_UINT32(6, q.coordinator_id);
    TEST_ASSERT_EQUAL_UINT64(1001, q.tick);
}

void test_membership_ignores_older_via_payload() {
    MembershipPayload p = {};
    p.epoch = 17;
    p.coordinator = 5;
    p.count = 3;
    p.members[0] = 1;
    p.members[1] = 3;
    p.members[2] = 5;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    TEST_ASSERT_TRUE(encode_membership(buf, sizeof(buf), n, sample_header(PKT_MEMBERSHIP), p));
    MembershipPayload q = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_membership(buf, n, dh, q));
    TEST_ASSERT_EQUAL_UINT64(17, q.epoch);
    TEST_ASSERT_EQUAL_UINT32(5, q.coordinator);
    TEST_ASSERT_EQUAL_UINT8(3, q.count);
    TEST_ASSERT_EQUAL_UINT32(3, q.members[1]);
}

void test_field_tick_and_delta_roundtrip() {
    FieldTickPayload t = {};
    t.tick = 88;
    t.dt = 0.5f;
    t.coordinator_id = 6;
    t.membership_epoch = 4;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    TEST_ASSERT_TRUE(encode_field_tick(buf, sizeof(buf), n, sample_header(PKT_FIELD_TICK), t));
    FieldTickPayload t2 = {};
    PacketHeader dh;
    TEST_ASSERT_TRUE(decode_field_tick(buf, n, dh, t2));
    TEST_ASSERT_EQUAL_UINT64(88, t2.tick);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, t2.dt);

    FieldState s;
    field_state_zero(s);
    s.information = 0.25f;
    s.signal = 0.5f;
    n = 0;
    TEST_ASSERT_TRUE(encode_state_delta(buf, sizeof(buf), n, sample_header(PKT_STATE_DELTA), s, 9));
    FieldState s2;
    field_state_zero(s2);
    uint32_t ver = 0;
    TEST_ASSERT_TRUE(decode_state_delta(buf, n, dh, s2, ver));
    TEST_ASSERT_EQUAL_UINT32(9, ver);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, s2.information);
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
    RUN_TEST(test_heartbeat_roundtrip);
    RUN_TEST(test_membership_ignores_older_via_payload);
    RUN_TEST(test_field_tick_and_delta_roundtrip);
    RUN_TEST(test_validate_rejects_bad_version_and_truncation);
    RUN_TEST(test_header_is_28_bytes);
    return UNITY_END();
}
