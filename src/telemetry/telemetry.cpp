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

static void print_c3json(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock) {
    Serial.print("C3JSON {");
    Serial.printf("\"type\":\"c3_swarm\",\"source_class\":\"physical\",\"synthetic\":false,");
    Serial.printf("\"node\":\"c3-%02lu\",\"body_id\":\"c3-%02lu\",\"body_type\":\"c3_swarm\",",
                  static_cast<unsigned long>(rt.self.node_id),
                  static_cast<unsigned long>(rt.self.node_id));
    Serial.printf("\"node_id\":%lu,\"boot_id\":%lu,\"hw\":%llu,",
                  static_cast<unsigned long>(rt.self.node_id),
                  static_cast<unsigned long>(rt.self.boot_id),
                  static_cast<unsigned long long>(rt.self.hardware_id));
    Serial.printf("\"phase\":\"%s\",\"coordinator\":%lu,\"epoch\":%llu,\"tick\":%llu,\"dt\":%.3f,",
                  phase_name(rt.phase),
                  static_cast<unsigned long>(rt.coordinator),
                  static_cast<unsigned long long>(rt.membership.epoch),
                  static_cast<unsigned long long>(clock.tick),
                  static_cast<double>(clock.dt));
    Serial.printf("\"temperature\":%.4f,\"information\":%.4f,\"energy\":%.4f,\"signal\":%.4f,\"version\":%lu,",
                  static_cast<double>(field.local.temperature),
                  static_cast<double>(field.local.information),
                  static_cast<double>(field.local.energy),
                  static_cast<double>(field.local.signal),
                  static_cast<unsigned long>(field.version));
    Serial.print("\"members\":[");
    for (uint8_t i = 0; i < rt.membership.count; ++i) {
        if (i) Serial.print(',');
        Serial.printf("%lu", static_cast<unsigned long>(rt.membership.members[i]));
    }
    Serial.print("],\"neighbors\":[");
    bool first = true;
    for (uint8_t i = 0; i < field.neighbor_count; ++i) {
        const NeighborField& nf = field.neighbors[i];
        if (!nf.valid) continue;
        if (!first) Serial.print(',');
        first = false;
        int rssi = 0;
        bool alive = false;
        for (uint8_t n = 0; n < rt.neighbors.count; ++n) {
            if (rt.neighbors.rows[n].node_id == nf.node_id) {
                rssi = rt.neighbors.rows[n].rssi;
                alive = rt.neighbors.rows[n].alive;
                break;
            }
        }
        Serial.printf("{\"node_id\":%lu,\"temperature\":%.4f,\"information\":%.4f,\"energy\":%.4f,\"signal\":%.4f,\"rssi\":%d,\"alive\":%s}",
                      static_cast<unsigned long>(nf.node_id),
                      static_cast<double>(nf.state.temperature),
                      static_cast<double>(nf.state.information),
                      static_cast<double>(nf.state.energy),
                      static_cast<double>(nf.state.signal),
                      rssi,
                      alive ? "true" : "false");
    }
    Serial.println("]}");
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
    print_c3json(rt, field, clock);
}

void telemetry_maybe(const SwarmRuntime& rt, const FieldStore& field, const TickClock& clock, uint32_t now_ms) {
    if (now_ms - last_telem_ms < SWARM_TELEMETRY_MS) {
        return;
    }
    last_telem_ms = now_ms;
    telemetry_print(rt, field, clock);
}

#endif
