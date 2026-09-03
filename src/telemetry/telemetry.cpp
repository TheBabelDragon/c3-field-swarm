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
    Serial.printf("\"node\":\"c3-%02lu\",\"body_id\":\"c3-%02lu\",\"body_type\":\"C3\",",
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
    for (uint8_t i = 0; i < rt.neighbors.count; ++i) {
        const NeighborRecord& row = rt.neighbors.rows[i];
        if (row.node_id == 0) continue;
        float temp = 0, info = 0, energy = 0, signal = 0;
        for (uint8_t f = 0; f < field.neighbor_count; ++f) {
            const NeighborField& nf = field.neighbors[f];
            if (nf.valid && nf.node_id == row.node_id) {
                temp = nf.state.temperature;
                info = nf.state.information;
                energy = nf.state.energy;
                signal = nf.state.signal;
                break;
            }
        }
        if (!first) Serial.print(',');
        first = false;
        Serial.printf("{\"node_id\":%lu,\"hw\":%llu,\"temperature\":%.4f,\"information\":%.4f,\"energy\":%.4f,\"signal\":%.4f,\"rssi\":%d,\"alive\":%s}",
                      static_cast<unsigned long>(row.node_id),
                      static_cast<unsigned long long>(row.hardware_id),
                      static_cast<double>(temp),
                      static_cast<double>(info),
                      static_cast<double>(energy),
                      static_cast<double>(signal),
                      static_cast<int>(row.rssi),
                      row.alive ? "true" : "false");
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
