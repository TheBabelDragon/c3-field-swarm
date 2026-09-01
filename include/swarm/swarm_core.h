#pragma once

#ifdef ARDUINO

#include "swarm/state.h"
#include "swarm/tick.h"
#include "field/field_state.h"
#include "transport/transport.h"

class SwarmCore {
public:
    void begin(Transport& transport, const NodeIdentity& self, uint32_t caps);
    void loop(uint32_t now_ms);
    void on_packet(const uint8_t* data, size_t length, int8_t rssi);
    void request_election(uint32_t now_ms);
    void inject(Channel ch, float value);
    void say_goodbye();

    SwarmRuntime& runtime();
    const SwarmRuntime& runtime() const;
    FieldStore& field();
    const FieldStore& field() const;
    TickClock& clock();

    void set_capabilities(uint32_t caps);

private:
    void send_hello();
    void send_hello_ack(NodeId dest);
    void send_heartbeat(uint32_t now_ms);
    void send_membership();
    void send_election();
    void send_digest();
    void send_state_delta(NodeId dest);
    void send_field_tick();
    void send_goodbye();

    void handle_hello(const PacketHeader& h, const uint8_t* buf, size_t len, int8_t rssi, uint32_t now_ms);
    void handle_heartbeat(const PacketHeader& h, const uint8_t* buf, size_t len, int8_t rssi, uint32_t now_ms);
    void handle_membership(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_election(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_digest(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_state_delta(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_field_tick(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_command(const PacketHeader& h, const uint8_t* buf, size_t len);
    void handle_goodbye(const PacketHeader& h);

    void rebuild_membership_from_neighbors();
    void maybe_elect(uint32_t now_ms);
    void apply_winner(NodeId winner, uint64_t epoch);
    void advance_field();
    void note_peer(NodeId id, BootId boot, int8_t rssi, uint64_t seq, uint32_t now_ms);

    Transport* transport_;
    SwarmRuntime rt_;
    TickClock clock_;
    FieldStore field_;
    uint32_t last_field_ms_;
    uint32_t last_digest_ms_;
};

#endif
