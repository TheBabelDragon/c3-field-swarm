#pragma once

#include <stdint.h>
#include <stddef.h>

#include "swarm/node_id.h"

class Transport {
public:
    virtual ~Transport() {}
    virtual bool begin() = 0;
    virtual bool send(NodeId destination, const uint8_t* data, size_t length) = 0;
    virtual void broadcast(const uint8_t* data, size_t length) = 0;
};

using PacketHandler = void (*)(const uint8_t* data, size_t length, int8_t rssi, void* user);

#ifdef ARDUINO
class EspNowTransport : public Transport {
public:
    bool begin() override;
    bool send(NodeId destination, const uint8_t* data, size_t length) override;
    void broadcast(const uint8_t* data, size_t length) override;
    void set_handler(PacketHandler handler, void* user);
    void map_peer(NodeId id, const uint8_t mac[6]);
    bool lookup_mac(NodeId id, uint8_t mac[6]) const;

private:
    struct Peer {
        NodeId id;
        uint8_t mac[6];
        bool used;
    };
    Peer peers_[SWARM_MAX_NODES];
};
#endif
