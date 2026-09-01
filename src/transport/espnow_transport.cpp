#include "transport/transport.h"

#ifdef ARDUINO

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

#include "swarm/fault_injection.h"

static PacketHandler g_handler = nullptr;
static void* g_user = nullptr;

static void on_recv(const uint8_t* mac, const uint8_t* data, int len) {
    (void)mac;
    if (g_handler == nullptr || data == nullptr || len <= 0) {
        return;
    }
    int8_t rssi = 0;
    g_handler(data, static_cast<size_t>(len), rssi, g_user);
}

static const uint8_t kBroadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool EspNowTransport::begin() {
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        peers_[i].used = false;
        peers_[i].id = 0;
        memset(peers_[i].mac, 0, 6);
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(SWARM_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_register_recv_cb(on_recv);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, kBroadcastMac, 6);
    peer.channel = SWARM_WIFI_CHANNEL;
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_STA;
    if (esp_now_is_peer_exist(kBroadcastMac)) {
        return true;
    }
    return esp_now_add_peer(&peer) == ESP_OK;
}

void EspNowTransport::set_handler(PacketHandler handler, void* user) {
    g_handler = handler;
    g_user = user;
}

void EspNowTransport::map_peer(NodeId id, const uint8_t mac[6]) {
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        if (peers_[i].used && peers_[i].id == id) {
            memcpy(peers_[i].mac, mac, 6);
            return;
        }
    }
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        if (!peers_[i].used) {
            peers_[i].used = true;
            peers_[i].id = id;
            memcpy(peers_[i].mac, mac, 6);
            return;
        }
    }
}

bool EspNowTransport::lookup_mac(NodeId id, uint8_t mac[6]) const {
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        if (peers_[i].used && peers_[i].id == id) {
            memcpy(mac, peers_[i].mac, 6);
            return true;
        }
    }
    return false;
}

bool EspNowTransport::send(NodeId destination, const uint8_t* data, size_t length) {
    if (destination == kBroadcastNodeId) {
        broadcast(data, length);
        return true;
    }
    uint8_t mac[6];
    if (!lookup_mac(destination, mac)) {
        broadcast(data, length);
        return true;
    }
    if (length > SWARM_MAX_PACKET) {
        return false;
    }
    return esp_now_send(mac, data, static_cast<int>(length)) == ESP_OK;
}

void EspNowTransport::broadcast(const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0 || length > SWARM_MAX_PACKET) {
        return;
    }
    esp_now_send(kBroadcastMac, data, static_cast<int>(length));
}

#endif
