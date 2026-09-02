#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef SWARM_PROTOCOL_VERSION
#define SWARM_PROTOCOL_VERSION 1
#endif

#ifndef SWARM_FIRMWARE_VERSION
#define SWARM_FIRMWARE_VERSION 1
#endif

#ifndef SWARM_MAX_NODES
#define SWARM_MAX_NODES 6
#endif

#ifndef SWARM_HEARTBEAT_MS
#define SWARM_HEARTBEAT_MS 500
#endif

#ifndef SWARM_NEIGHBOR_TIMEOUT_MS
#define SWARM_NEIGHBOR_TIMEOUT_MS 2500
#endif

#ifndef SWARM_COORDINATOR_TIMEOUT_MS
#define SWARM_COORDINATOR_TIMEOUT_MS 2500
#endif

#ifndef SWARM_HELLO_PERIOD_MS
#define SWARM_HELLO_PERIOD_MS 750
#endif

#ifndef SWARM_DISCOVERY_MS
#define SWARM_DISCOVERY_MS 2250
#endif

#ifndef SWARM_TICK_DT
#define SWARM_TICK_DT 0.5f
#endif

#ifndef SWARM_MAX_PACKET
#define SWARM_MAX_PACKET 250
#endif

#ifndef SWARM_MAX_DELTAS
#define SWARM_MAX_DELTAS 16
#endif

#ifndef SWARM_WIFI_CHANNEL
#define SWARM_WIFI_CHANNEL 1
#endif

#ifndef SWARM_DIFFUSION_RATE
#define SWARM_DIFFUSION_RATE 0.25f
#endif

#ifndef SWARM_DECAY_RATE
#define SWARM_DECAY_RATE 0.08f
#endif

#ifndef SWARM_TELEMETRY_MS
#define SWARM_TELEMETRY_MS 2000
#endif

static constexpr uint32_t kBroadcastNodeId = 0xFFFFFFFFu;
static constexpr uint8_t kPacketVersion = SWARM_PROTOCOL_VERSION;
