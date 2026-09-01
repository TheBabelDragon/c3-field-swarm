#include "swarm/node_id.h"

#ifdef ARDUINO

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <esp_mac.h>
#include <esp_random.h>

static Preferences prefs;

static HardwareId mac_to_hardware_id() {
    uint8_t mac[6] = {0};
    WiFi.macAddress(mac);
    HardwareId id = 0;
    for (int i = 0; i < 6; ++i) {
        id = (id << 8) | mac[i];
    }
    return id;
}

static NodeId derive_default_node_id(HardwareId hw) {
    uint32_t mix = static_cast<uint32_t>(hw) ^ static_cast<uint32_t>(hw >> 32);
    mix ^= mix >> 16;
    NodeId id = (mix % SWARM_MAX_NODES) + 1;
    return id;
}

bool node_identity_begin(NodeIdentity& out) {
    out.hardware_id = mac_to_hardware_id();
    out.boot_id = esp_random();
    if (out.boot_id == 0) {
        out.boot_id = 1;
    }

    prefs.begin("c3swarm", false);
    uint32_t stored = prefs.getUInt("node_id", 0);
    if (stored >= 1 && stored <= SWARM_MAX_NODES) {
        out.node_id = stored;
    } else {
        out.node_id = derive_default_node_id(out.hardware_id);
        prefs.putUInt("node_id", out.node_id);
    }
    prefs.end();
    return true;
}

void node_identity_force_id(NodeIdentity& id, NodeId node_id) {
    if (node_id < 1 || node_id > SWARM_MAX_NODES) {
        return;
    }
    id.node_id = node_id;
    prefs.begin("c3swarm", false);
    prefs.putUInt("node_id", node_id);
    prefs.end();
}

#endif
