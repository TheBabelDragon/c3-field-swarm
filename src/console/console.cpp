#include "console/console.h"

#ifdef ARDUINO

#include <Arduino.h>
#include <string.h>

#include "telemetry/telemetry.h"
#include "field/field_state.h"
#include "swarm/protocol.h"

static char line[96];
static size_t line_len = 0;

static void print_help() {
    Serial.println("commands:");
    Serial.println("  help       this list");
    Serial.println("  status     identity / membership / tick");
    Serial.println("  nodes      membership view");
    Serial.println("  state      local field channels");
    Serial.println("  tick       current field tick");
    Serial.println("  neighbors  neighbor table");
    Serial.println("  elect      force coordinator election");
    Serial.println("  inject <ch> <value>   ch=temp|info|energy|signal");
    Serial.println("  id <n>     persist logical node_id 1-6 and reboot");
    Serial.println("  reset      reboot");
}

static void print_nodes(const SwarmRuntime& rt) {
    for (uint8_t i = 0; i < rt.membership.count; ++i) {
        Serial.printf("%02lu ONLINE\n", static_cast<unsigned long>(rt.membership.members[i]));
    }
    Serial.printf("coordinator: %02lu\n", static_cast<unsigned long>(rt.coordinator));
    Serial.printf("epoch: %llu\n", static_cast<unsigned long long>(rt.membership.epoch));
}

static void print_neighbors(const SwarmRuntime& rt) {
    Serial.printf("neighbors: %u\n", rt.neighbors.count);
    for (uint8_t i = 0; i < rt.neighbors.count; ++i) {
        const NeighborRecord& r = rt.neighbors.rows[i];
        Serial.printf("  %02lu boot=%lu seq=%llu rssi=%d caps=0x%lx %s\n",
                      static_cast<unsigned long>(r.node_id),
                      static_cast<unsigned long>(r.boot_id),
                      static_cast<unsigned long long>(r.last_sequence),
                      static_cast<int>(r.rssi),
                      static_cast<unsigned long>(r.capabilities),
                      r.alive ? "alive" : "stale");
    }
}

static Channel parse_channel(const char* name) {
    if (strcmp(name, "temp") == 0 || strcmp(name, "temperature") == 0) return CH_TEMPERATURE;
    if (strcmp(name, "info") == 0 || strcmp(name, "information") == 0) return CH_INFORMATION;
    if (strcmp(name, "energy") == 0) return CH_ENERGY;
    if (strcmp(name, "signal") == 0) return CH_SIGNAL;
    return CH_COUNT;
}

static void handle_line(SwarmCore& core, char* text) {
    while (*text == ' ') ++text;
    if (*text == 0) return;

    char* cmd = strtok(text, " ");
    if (cmd == nullptr) return;

    if (strcmp(cmd, "help") == 0) {
        print_help();
    } else if (strcmp(cmd, "status") == 0) {
        telemetry_print(core.runtime(), core.field(), core.clock());
    } else if (strcmp(cmd, "nodes") == 0) {
        print_nodes(core.runtime());
    } else if (strcmp(cmd, "state") == 0) {
        const FieldState& s = core.field().local;
        Serial.printf("temperature=%.4f information=%.4f energy=%.4f signal=%.4f version=%lu\n",
                      static_cast<double>(s.temperature),
                      static_cast<double>(s.information),
                      static_cast<double>(s.energy),
                      static_cast<double>(s.signal),
                      static_cast<unsigned long>(core.runtime().state_version));
    } else if (strcmp(cmd, "tick") == 0) {
        Serial.printf("tick=%llu dt=%.3f\n",
                      static_cast<unsigned long long>(core.clock().tick),
                      static_cast<double>(core.clock().dt));
    } else if (strcmp(cmd, "neighbors") == 0) {
        print_neighbors(core.runtime());
    } else if (strcmp(cmd, "elect") == 0) {
        core.request_election(millis());
        Serial.println("election requested");
    } else if (strcmp(cmd, "inject") == 0) {
        char* chs = strtok(nullptr, " ");
        char* vs = strtok(nullptr, " ");
        if (!chs || !vs) {
            Serial.println("usage: inject <temp|info|energy|signal> <value>");
            return;
        }
        Channel ch = parse_channel(chs);
        if (ch == CH_COUNT) {
            Serial.println("unknown channel");
            return;
        }
        core.inject(ch, static_cast<float>(atof(vs)));
        Serial.println("injected");
    } else if (strcmp(cmd, "id") == 0) {
        char* ns = strtok(nullptr, " ");
        if (!ns) {
            Serial.println("usage: id <1-6>");
            return;
        }
        NodeId nid = static_cast<NodeId>(atoi(ns));
        node_identity_force_id(core.runtime().self, nid);
        Serial.printf("node_id persisted as %02lu, rebooting\n", static_cast<unsigned long>(nid));
        delay(100);
        ESP.restart();
    } else if (strcmp(cmd, "reset") == 0) {
        core.say_goodbye();
        delay(50);
        ESP.restart();
    } else {
        Serial.printf("unknown: %s\n", cmd);
    }
}

void console_begin() {
    line_len = 0;
    Serial.println("c3-field-swarm console. type help");
    Serial.print("> ");
}

void console_poll(SwarmCore& core) {
    while (Serial.available() > 0) {
        char c = static_cast<char>(Serial.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            line[line_len] = 0;
            Serial.println();
            handle_line(core, line);
            line_len = 0;
            Serial.print("> ");
            continue;
        }
        if (line_len + 1 < sizeof(line)) {
            line[line_len++] = c;
        }
    }
}

#endif
