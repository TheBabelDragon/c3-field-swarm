#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "config.h"
#include "swarm/node_id.h"

enum PacketType : uint8_t {
    PKT_HELLO         = 1,
    PKT_HELLO_ACK     = 2,
    PKT_HEARTBEAT     = 3,
    PKT_MEMBERSHIP    = 4,
    PKT_STATE_DIGEST  = 5,
    PKT_STATE_DELTA   = 6,
    PKT_FIELD_TICK    = 7,
    PKT_COMMAND       = 8,
    PKT_COMMAND_ACK   = 9,
    PKT_ELECTION      = 10,
    PKT_ELECTION_ACK  = 11,
    PKT_TELEMETRY     = 12,
    PKT_GOODBYE       = 13,
    PKT_STATE_REQUEST = 14,
};

struct PacketHeader {
    uint8_t version;
    uint8_t type;
    uint16_t length;
    uint32_t sender_id;
    uint32_t boot_id;
    uint64_t sequence;
    uint64_t tick;
};

static constexpr size_t kPacketHeaderSize = 28;

struct HelloPayload {
    uint64_t hardware_id;
    uint32_t node_id;
    uint32_t boot_id;
    uint8_t protocol_version;
    uint8_t firmware_version;
    uint16_t reserved;
    uint32_t capabilities;
};

struct HeartbeatPayload {
    uint32_t node_id;
    uint32_t boot_id;
    uint64_t tick;
    uint32_t uptime_ms;
    uint32_t free_heap;
    uint32_t local_state_version;
    uint32_t coordinator_id;
};

struct MembershipPayload {
    uint64_t epoch;
    uint32_t coordinator;
    uint8_t count;
    uint8_t reserved[3];
    uint32_t members[SWARM_MAX_NODES];
};

struct ElectionPayload {
    uint64_t epoch;
    uint32_t candidate;
    uint32_t candidate_boot;
    uint8_t term;
    uint8_t reserved[3];
};

struct StateDigestPayload {
    uint32_t state_version;
    uint32_t coordinator_id;
    uint64_t tick;
    uint32_t checksum;
};

struct FieldTickPayload {
    uint64_t tick;
    float dt;
    uint32_t coordinator_id;
    uint64_t membership_epoch;
};

enum Channel : uint8_t {
    CH_TEMPERATURE = 0,
    CH_INFORMATION = 1,
    CH_ENERGY      = 2,
    CH_SIGNAL      = 3,
    CH_COUNT       = 4,
};

struct FieldDeltaWire {
    uint32_t source_node;
    uint64_t tick;
    uint8_t channel;
    uint8_t reserved[3];
    float old_value;
    float new_value;
};

struct CommandPayload {
    uint32_t command_id;
    uint32_t target_node;
    uint8_t opcode;
    uint8_t reserved[3];
    float arg0;
    float arg1;
};

enum CommandOpcode : uint8_t {
    CMD_NONE     = 0,
    CMD_INJECT   = 1,
    CMD_ELECT    = 2,
    CMD_RESET    = 3,
    CMD_SET_RATE = 4,
};

const char* packet_type_name(uint8_t type);
bool packet_type_known(uint8_t type);

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdio.h>
#endif
