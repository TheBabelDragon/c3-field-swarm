#include <Arduino.h>

#include "config.h"
#include "swarm/node_id.h"
#include "swarm/capabilities.h"
#include "swarm/swarm_core.h"
#include "transport/transport.h"
#include "telemetry/telemetry.h"
#include "console/console.h"

static EspNowTransport transport;
static SwarmCore swarm;

static void on_rx(const uint8_t* data, size_t length, int8_t rssi, const uint8_t* mac, void* user) {
    (void)user;
    swarm.on_packet(data, length, rssi, mac);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    const uint32_t wait_start = millis();
    while (!Serial && (millis() - wait_start) < 2500) {
        delay(10);
    }
    delay(150);

    NodeIdentity self;
    if (!node_identity_begin(self)) {
        Serial.println("[C3] identity failed");
        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println("========================================");
    Serial.println(" c3-field-swarm");
    Serial.println(" ESP32-C3 wireless swarm substrate");
    Serial.println("========================================");
    Serial.printf(" hardware_id=%llx\n", static_cast<unsigned long long>(self.hardware_id));
    Serial.printf(" node_id=%02lu\n", static_cast<unsigned long>(self.node_id));
    Serial.printf(" boot_id=%lu\n", static_cast<unsigned long>(self.boot_id));
    Serial.printf(" protocol=%u firmware=%u\n", SWARM_PROTOCOL_VERSION, SWARM_FIRMWARE_VERSION);

    if (!transport.begin()) {
        Serial.println("[C3] ESP-NOW init failed");
        while (true) {
            delay(1000);
        }
    }
    transport.set_handler(on_rx, nullptr);

    swarm.begin(transport, self, default_capabilities());
    console_begin();
    telemetry_print(swarm.runtime(), swarm.field(), swarm.clock());
}

void loop() {
    uint32_t now = millis();
    swarm.loop(now);
    telemetry_maybe(swarm.runtime(), swarm.field(), swarm.clock(), now);
    console_poll(swarm);
    delay(5);
}
