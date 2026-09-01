#include "telemetry/telemetry.h"

#ifdef ARDUINO

#include <Arduino.h>

static uint32_t last_telem_ms = 0;

static const char* phase_name(SwarmPhase p) {
    switch (p) {
        case PHASE_BOOT: return "BOOT";
        case PHASE_INITIALIZE: return "INITIALIZE";
        case PHASE_DISCOVERY: return "DISCOVERY";
        case PHASE_MEMBERSHIP: return "MEMBERSHIP";
        case PHASE_ELECTION: return "ELECTION";
        case PHASE_RUN: return "RUN";
        default: return "?";
    }
}

void telemetry_print(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock) {
    Serial.printf("[C3] node=%02lu boot=%lu hw=%llx phase=%s\n",
                  static_cast<unsigned long>(rt.self.node_id),
                  static_cast<unsigned long>(rt.self.boot_id),
                  static_cast<unsigned long long>(rt.self.hardware_id),
                  phase_name(rt.phase));
    Serial.printf("[C3] members=%u coordinator=%02lu epoch=%llu\n",
                  rt.membership.count,
                  static_cast<unsigned long>(rt.coordinator),
                  static_cast<unsigned long long>(rt.membership.epoch));
    Serial.printf("[C3] tick=%llu dt=%.3f\n",
                  static_cast<unsigned long long>(clock.tick),
                  static_cast<double>(clock.dt));
    Serial.printf("[C3] neighbors=%u heap=%u\n",
                  neighbor_alive_count(rt.neighbors),
                  ESP.getFreeHeap());
    Serial.printf("[C3] temperature=%.2f\n", static_cast<double>(field.local.temperature));
    Serial.printf("[C3] information=%.2f\n", static_cast<double>(field.local.information));
    Serial.printf("[C3] energy=%.2f\n", static_cast<double>(field.local.energy));
    Serial.printf("[C3] signal=%.2f\n", static_cast<double>(field.local.signal));
}

void telemetry_maybe(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock, uint32_t now_ms) {
    if (now_ms - last_telem_ms < SWARM_TELEMETRY_MS) {
        return;
    }
    last_telem_ms = now_ms;
    telemetry_print(rt, field, clock);
}

#endif
