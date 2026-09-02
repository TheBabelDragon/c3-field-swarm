#include "swarm/swarm_core.h"

#ifdef ARDUINO

#include <Arduino.h>

#include "swarm/election.h"
#include "swarm/protocol.h"
#include "swarm/join.h"
#include "transport/packet_codec.h"
#include "field/field_view.h"
#include "field/field_delta.h"
#include "field/field_system.h"
#include "systems/diffusion.h"
#include "systems/information_decay.h"
#include "systems/aggregation.h"

static uint32_t tick_period_ms() {
    uint32_t ms = static_cast<uint32_t>(SWARM_TICK_DT * 1000.0f);
    return ms == 0 ? SWARM_HEARTBEAT_MS : ms;
}

void SwarmCore::begin(Transport& transport, const NodeIdentity& self, uint32_t caps) {
    transport_ = &transport;
    swarm_runtime_begin(rt_, self, caps);
    tick_clock_init(clock_, SWARM_TICK_DT);
    field_store_init(field_);
    last_field_ms_ = 0;
    last_digest_ms_ = 0;
    discovery_until_ms_ = millis() + SWARM_DISCOVERY_MS;
    rt_.phase = PHASE_DISCOVERY;
    send_hello();
}

void SwarmCore::set_capabilities(uint32_t caps) {
    rt_.capabilities = caps;
}

SwarmRuntime& SwarmCore::runtime() {
    return rt_;
}

const SwarmRuntime& SwarmCore::runtime() const {
    return rt_;
}

FieldStore& SwarmCore::field() {
    return field_;
}

const FieldStore& SwarmCore::field() const {
    return field_;
}

TickClock& SwarmCore::clock() {
    return clock_;
}

void SwarmCore::inject(Channel ch, float value) {
    field_channel_set(field_.local, ch, value);
    field_.version++;
    rt_.state_version = field_.version;
    send_digest();
}

void SwarmCore::request_election(uint32_t now_ms) {
    rt_.coordinator = 0;
    rt_.coordinator_is_self = false;
    rt_.membership.coordinator = 0;
    rt_.phase = PHASE_ELECTION;
    rt_.last_coord_seen_ms = 0;
    maybe_elect(now_ms);
}

void SwarmCore::say_goodbye() {
    send_goodbye();
}

void SwarmCore::loop(uint32_t now_ms) {
    if (transport_ == nullptr) {
        return;
    }

    uint8_t expired = neighbor_expire(rt_.neighbors, now_ms, SWARM_NEIGHBOR_TIMEOUT_MS);
    if (expired > 0) {
        rebuild_membership_from_neighbors();
        rt_.membership.epoch++;
        field_store_retain_members(field_, rt_.membership.members, rt_.membership.count);
        if (!rt_.coordinator_is_self && rt_.coordinator != 0) {
            NeighborRecord* rec = neighbor_find(rt_.neighbors, rt_.coordinator);
            if (rec == nullptr || !rec->alive) {
                rt_.coordinator = 0;
                rt_.membership.coordinator = 0;
            }
        }
        if (rt_.coordinator_is_self) {
            send_membership();
        }
    }

    if (now_ms - rt_.last_hello_ms >= SWARM_HELLO_PERIOD_MS || rt_.last_hello_ms == 0) {
        send_hello();
        rt_.last_hello_ms = now_ms;
    }

    if (now_ms - rt_.last_heartbeat_ms >= SWARM_HEARTBEAT_MS || rt_.last_heartbeat_ms == 0) {
        send_heartbeat(now_ms);
        rt_.last_heartbeat_ms = now_ms;
    }

    if (!discovering(now_ms)) {
        maybe_elect(now_ms);
        if (rt_.phase == PHASE_DISCOVERY) {
            rt_.phase = (rt_.coordinator != 0) ? PHASE_RUN : PHASE_MEMBERSHIP;
            rebuild_membership_from_neighbors();
        }
    }

    const uint32_t period = tick_period_ms();
    if (rt_.coordinator_is_self) {
        if (last_field_ms_ == 0 || now_ms - last_field_ms_ >= period) {
            tick_clock_advance_local(clock_);
            rt_.tick = clock_.tick;
            send_field_tick();
            advance_field();
            last_field_ms_ = now_ms;
        }
        if (last_digest_ms_ == 0 || now_ms - last_digest_ms_ >= SWARM_HEARTBEAT_MS * 2) {
            send_digest();
            last_digest_ms_ = now_ms;
        }
    } else if (last_field_ms_ != 0 && now_ms - last_field_ms_ >= period * 2) {
        tick_clock_advance_local(clock_);
        rt_.tick = clock_.tick;
        advance_field();
        last_field_ms_ = now_ms;
    }

    if (rt_.phase == PHASE_DISCOVERY && neighbor_alive_count(rt_.neighbors) > 0) {
        rt_.phase = PHASE_MEMBERSHIP;
        rebuild_membership_from_neighbors();
    }
    if (rt_.coordinator != 0 && rt_.phase != PHASE_ELECTION) {
        rt_.phase = PHASE_RUN;
    }
}

void SwarmCore::on_packet(const uint8_t* data, size_t length, int8_t rssi, const uint8_t* mac) {
    PacketHeader h;
    if (!validate_packet(data, length, h)) {
        return;
    }

    if (mac != nullptr && transport_ != nullptr && h.sender_id != rt_.self.node_id) {
        transport_->remember_peer(h.sender_id, mac);
    }

    const uint32_t now_ms = millis();

    if (h.sender_id == rt_.self.node_id) {
        if (h.type == PKT_HELLO || h.type == PKT_HELLO_ACK) {
            handle_hello(h, data, length, rssi, now_ms);
        }
        return;
    }

    NeighborRecord* rec = neighbor_find(rt_.neighbors, h.sender_id);
    if (rec != nullptr && rec->boot_id == h.boot_id && h.sequence != 0 && h.sequence <= rec->last_sequence) {
        return;
    }

    switch (h.type) {
        case PKT_HELLO:
        case PKT_HELLO_ACK:
            handle_hello(h, data, length, rssi, now_ms);
            break;
        case PKT_HEARTBEAT:
            handle_heartbeat(h, data, length, rssi, now_ms);
            break;
        case PKT_MEMBERSHIP:
            handle_membership(h, data, length);
            break;
        case PKT_ELECTION:
        case PKT_ELECTION_ACK:
            handle_election(h, data, length);
            break;
        case PKT_STATE_DIGEST:
            handle_digest(h, data, length);
            break;
        case PKT_STATE_DELTA:
            handle_state_delta(h, data, length);
            break;
        case PKT_FIELD_TICK:
            handle_field_tick(h, data, length);
            break;
        case PKT_COMMAND:
            handle_command(h, data, length);
            break;
        case PKT_GOODBYE:
            handle_goodbye(h);
            break;
        case PKT_STATE_REQUEST:
            send_state_delta(h.sender_id);
            note_peer(h.sender_id, h.boot_id, rssi, h.sequence, now_ms);
            break;
        default:
            break;
    }
}

PacketHeader SwarmCore::make_header(uint8_t type) {
    PacketHeader h;
    h.version = kPacketVersion;
    h.type = type;
    h.length = 0;
    h.sender_id = rt_.self.node_id;
    h.boot_id = rt_.self.boot_id;
    h.sequence = rt_.sequence++;
    h.tick = clock_.tick;
    return h;
}

void SwarmCore::tx(const uint8_t* buf, size_t n, NodeId dest) {
    if (transport_ == nullptr || buf == nullptr || n == 0) {
        return;
    }
    if (dest == kBroadcastNodeId) {
        transport_->broadcast(buf, n);
    } else {
        transport_->send(dest, buf, n);
    }
}

void SwarmCore::send_hello() {
    HelloPayload p = {};
    p.hardware_id = rt_.self.hardware_id;
    p.node_id = rt_.self.node_id;
    p.boot_id = rt_.self.boot_id;
    p.protocol_version = SWARM_PROTOCOL_VERSION;
    p.firmware_version = SWARM_FIRMWARE_VERSION;
    p.reserved = 0;
    p.capabilities = rt_.capabilities;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_hello(buf, sizeof(buf), n, make_header(PKT_HELLO), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_hello_ack(NodeId dest) {
    HelloPayload p = {};
    p.hardware_id = rt_.self.hardware_id;
    p.node_id = rt_.self.node_id;
    p.boot_id = rt_.self.boot_id;
    p.protocol_version = SWARM_PROTOCOL_VERSION;
    p.firmware_version = SWARM_FIRMWARE_VERSION;
    p.reserved = 0;
    p.capabilities = rt_.capabilities;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_hello(buf, sizeof(buf), n, make_header(PKT_HELLO_ACK), p)) {
        return;
    }
    tx(buf, n, dest);
}

void SwarmCore::send_heartbeat(uint32_t now_ms) {
    HeartbeatPayload p = {};
    p.node_id = rt_.self.node_id;
    p.boot_id = rt_.self.boot_id;
    p.tick = clock_.tick;
    p.uptime_ms = now_ms;
    p.free_heap = ESP.getFreeHeap();
    p.local_state_version = rt_.state_version;
    p.coordinator_id = rt_.coordinator;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_heartbeat(buf, sizeof(buf), n, make_header(PKT_HEARTBEAT), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_membership() {
    MembershipPayload p = {};
    p.epoch = rt_.membership.epoch;
    p.coordinator = rt_.coordinator;
    p.count = rt_.membership.count;
    p.reserved[0] = p.reserved[1] = p.reserved[2] = 0;
    for (uint8_t i = 0; i < SWARM_MAX_NODES; ++i) {
        p.members[i] = (i < rt_.membership.count) ? rt_.membership.members[i] : 0;
    }
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_membership(buf, sizeof(buf), n, make_header(PKT_MEMBERSHIP), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_election() {
    ElectionPayload p = {};
    p.epoch = rt_.membership.epoch;
    p.candidate = rt_.coordinator ? rt_.coordinator : rt_.self.node_id;
    p.candidate_boot = rt_.self.boot_id;
    p.term = 1;
    p.reserved[0] = p.reserved[1] = p.reserved[2] = 0;
    NeighborRecord* rec = neighbor_find(rt_.neighbors, p.candidate);
    if (p.candidate == rt_.self.node_id) {
        p.candidate_boot = rt_.self.boot_id;
    } else if (rec != nullptr) {
        p.candidate_boot = rec->boot_id;
    }
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_election(buf, sizeof(buf), n, make_header(PKT_ELECTION), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_digest() {
    StateDigestPayload p = {};
    p.state_version = rt_.state_version;
    p.coordinator_id = rt_.coordinator;
    p.tick = clock_.tick;
    p.checksum = field_state_checksum(field_.local, clock_.tick, field_.version);
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_digest(buf, sizeof(buf), n, make_header(PKT_STATE_DIGEST), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_state_delta(NodeId dest) {
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_state_delta(buf, sizeof(buf), n, make_header(PKT_STATE_DELTA), field_.local, field_.version)) {
        return;
    }
    tx(buf, n, dest);
}

void SwarmCore::send_field_tick() {
    FieldTickPayload p = {};
    p.tick = clock_.tick;
    p.dt = clock_.dt;
    p.coordinator_id = rt_.self.node_id;
    p.membership_epoch = rt_.membership.epoch;
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_field_tick(buf, sizeof(buf), n, make_header(PKT_FIELD_TICK), p)) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::send_goodbye() {
    uint8_t buf[SWARM_MAX_PACKET];
    size_t n = 0;
    if (!encode_goodbye(buf, sizeof(buf), n, make_header(PKT_GOODBYE))) {
        return;
    }
    tx(buf, n);
}

void SwarmCore::note_peer(NodeId id, BootId boot, int8_t rssi, uint64_t seq, uint32_t now_ms) {
    if (id == 0 || id == rt_.self.node_id) {
        return;
    }
    NeighborRecord* rec = neighbor_upsert(rt_.neighbors, id);
    if (rec == nullptr) {
        return;
    }
    rec->boot_id = boot;
    rec->last_seen_ms = now_ms;
    rec->last_sequence = seq;
    rec->rssi = rssi;
    rec->last_tick = clock_.tick;
    rec->alive = true;
    membership_add(rt_.membership, id);
}

void SwarmCore::rebuild_membership_from_neighbors() {
    Membership next;
    membership_clear(next);
    next.epoch = rt_.membership.epoch;
    next.coordinator = rt_.coordinator;
    membership_add(next, rt_.self.node_id);
    for (uint8_t i = 0; i < rt_.neighbors.count; ++i) {
        if (rt_.neighbors.rows[i].alive) {
            membership_add(next, rt_.neighbors.rows[i].node_id);
        }
    }
    rt_.membership = next;
}

void SwarmCore::apply_winner(NodeId winner, uint64_t epoch) {
    rt_.coordinator = winner;
    rt_.membership.coordinator = winner;
    rt_.membership.epoch = epoch;
    rt_.coordinator_is_self = (winner == rt_.self.node_id);
    rt_.phase = PHASE_RUN;
    if (rt_.coordinator_is_self) {
        rt_.last_coord_seen_ms = millis();
        send_membership();
    }
}

bool SwarmCore::discovering(uint32_t now_ms) const {
    return join_in_discovery(now_ms, discovery_until_ms_);
}

void SwarmCore::adopt_coordinator(NodeId coord, uint64_t epoch, uint32_t now_ms) {
    if (coord == 0 || coord == rt_.self.node_id) {
        return;
    }
    rt_.coordinator = coord;
    rt_.membership.coordinator = coord;
    rt_.coordinator_is_self = false;
    if (epoch > rt_.membership.epoch) {
        rt_.membership.epoch = epoch;
    }
    rt_.last_coord_seen_ms = now_ms;
    rt_.phase = PHASE_RUN;
}

void SwarmCore::resolve_duplicate_id(HardwareId other_hw) {
    if (!join_should_yield_node_id(rt_.self.hardware_id, other_hw)) {
        send_hello();
        return;
    }
    NodeId taken[SWARM_MAX_NODES];
    uint8_t n = 0;
    taken[n++] = rt_.self.node_id;
    for (uint8_t i = 0; i < rt_.neighbors.count && n < SWARM_MAX_NODES; ++i) {
        taken[n++] = rt_.neighbors.rows[i].node_id;
    }
    NodeId next = join_next_free_id(taken, n);
    if (next == 0) {
        Serial.println("[C3] node_id collision and swarm is full");
        return;
    }
    Serial.printf("[C3] node_id collision on %02lu, persisting %02lu\n",
                  static_cast<unsigned long>(rt_.self.node_id),
                  static_cast<unsigned long>(next));
    send_goodbye();
    membership_remove(rt_.membership, rt_.self.node_id);
    rt_.self.node_id = next;
    node_identity_force_id(rt_.self, next);
    membership_add(rt_.membership, next);
    send_hello();
}

void SwarmCore::maybe_elect(uint32_t now_ms) {
    if (!should_start_election(rt_, now_ms)) {
        return;
    }
    rt_.phase = PHASE_ELECTION;
    ElectionResult r = elect_from_membership(rt_.membership, rt_.neighbors, rt_.self, rt_.capabilities);
    if (!r.valid) {
        return;
    }
    apply_winner(r.winner, rt_.membership.epoch + 1);
    send_election();
}

void SwarmCore::advance_field() {
    DiffusionSystem diffusion;
    InformationDecaySystem decay;
    AggregationSystem agg;
    FieldSystem* systems[3] = {&decay, &diffusion, &agg};
    FieldView view;
    field_view_freeze(view, field_, rt_.self.node_id, clock_.tick, clock_.dt);
    FieldDeltaList applied;
    field_run_systems(view, systems, 3, field_.local, applied);
    if (applied.count > 0) {
        field_.version++;
        rt_.state_version = field_.version;
    }
}

void SwarmCore::handle_hello(const PacketHeader& h, const uint8_t* buf, size_t len, int8_t rssi, uint32_t now_ms) {
    HelloPayload p = {};
    PacketHeader decoded;
    if (!decode_hello(buf, len, decoded, p)) {
        return;
    }
    if (p.node_id == rt_.self.node_id && p.hardware_id != rt_.self.hardware_id) {
        resolve_duplicate_id(p.hardware_id);
        return;
    }
    if (h.sender_id == rt_.self.node_id) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, rssi, h.sequence, now_ms);
    NeighborRecord* rec = neighbor_find(rt_.neighbors, h.sender_id);
    if (rec != nullptr) {
        rec->hardware_id = p.hardware_id;
        rec->capabilities = p.capabilities ? p.capabilities : CAP_COORD_ELIGIBLE;
    }
    if (h.type == PKT_HELLO) {
        send_hello_ack(h.sender_id);
        if (rt_.coordinator_is_self) {
            send_membership();
        }
    }
}

void SwarmCore::handle_heartbeat(const PacketHeader& h, const uint8_t* buf, size_t len, int8_t rssi, uint32_t now_ms) {
    HeartbeatPayload p = {};
    PacketHeader decoded;
    if (!decode_heartbeat(buf, len, decoded, p)) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, rssi, h.sequence, now_ms);
    NeighborRecord* rec = neighbor_find(rt_.neighbors, h.sender_id);
    if (rec != nullptr) {
        rec->state_version = p.local_state_version;
        rec->last_tick = p.tick;
    }
    if (p.coordinator_id != 0) {
        if (join_should_adopt(rt_.self.node_id,
                              rt_.membership.count,
                              rt_.membership.epoch,
                              rt_.coordinator,
                              discovering(now_ms),
                              p.coordinator_id,
                              2,
                              rt_.membership.epoch)) {
            adopt_coordinator(p.coordinator_id, rt_.membership.epoch, now_ms);
        }
        if (p.coordinator_id == rt_.coordinator) {
            rt_.last_coord_seen_ms = now_ms;
        }
    }
}

void SwarmCore::handle_membership(const PacketHeader& h, const uint8_t* buf, size_t len) {
    MembershipPayload p = {};
    PacketHeader decoded;
    if (!decode_membership(buf, len, decoded, p)) {
        return;
    }
    const uint32_t now_ms = millis();
    const uint64_t local_epoch = rt_.membership.epoch;
    const bool adopt = join_should_adopt(rt_.self.node_id,
                                         rt_.membership.count,
                                         local_epoch,
                                         rt_.coordinator,
                                         discovering(now_ms),
                                         p.coordinator,
                                         p.count,
                                         p.epoch);
    if (!adopt && p.epoch < local_epoch) {
        return;
    }
    Membership m;
    membership_clear(m);
    m.epoch = p.epoch >= local_epoch ? p.epoch : local_epoch;
    m.coordinator = p.coordinator;
    uint8_t n = p.count;
    if (n > SWARM_MAX_NODES) {
        n = SWARM_MAX_NODES;
    }
    for (uint8_t i = 0; i < n; ++i) {
        if (p.members[i] != 0) {
            membership_add(m, p.members[i]);
        }
    }
    membership_add(m, rt_.self.node_id);
    rt_.membership = m;
    if (p.coordinator != 0 && p.coordinator != rt_.self.node_id && adopt) {
        adopt_coordinator(p.coordinator, p.epoch, now_ms);
    } else {
        rt_.coordinator = p.coordinator;
        rt_.coordinator_is_self = (p.coordinator == rt_.self.node_id);
        if (p.coordinator != 0) {
            rt_.last_coord_seen_ms = now_ms;
            rt_.phase = PHASE_RUN;
        }
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, now_ms);
}

void SwarmCore::handle_election(const PacketHeader& h, const uint8_t* buf, size_t len) {
    ElectionPayload p = {};
    PacketHeader decoded;
    if (!decode_election(buf, len, decoded, p)) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, millis());
    if (rt_.coordinator != 0 && !should_start_election(rt_, millis())) {
        return;
    }
    ElectionResult local = elect_from_membership(rt_.membership, rt_.neighbors, rt_.self, rt_.capabilities);
    NodeId winner = local.valid ? local.winner : p.candidate;
    uint64_t epoch = p.epoch > rt_.membership.epoch ? p.epoch : rt_.membership.epoch + 1;
    apply_winner(winner, epoch);
}

void SwarmCore::handle_digest(const PacketHeader& h, const uint8_t* buf, size_t len) {
    StateDigestPayload p = {};
    PacketHeader decoded;
    if (!decode_digest(buf, len, decoded, p)) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, millis());
    uint32_t local_ck = field_state_checksum(field_.local, clock_.tick, field_.version);
    if (p.state_version != rt_.state_version || p.checksum != local_ck) {
        send_state_delta(h.sender_id);
    }
}

void SwarmCore::handle_state_delta(const PacketHeader& h, const uint8_t* buf, size_t len) {
    FieldState remote;
    field_state_zero(remote);
    uint32_t version = 0;
    PacketHeader decoded;
    if (!decode_state_delta(buf, len, decoded, remote, version)) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, millis());
    field_store_set_neighbor(field_, h.sender_id, remote, version);
    NeighborRecord* rec = neighbor_find(rt_.neighbors, h.sender_id);
    if (rec != nullptr) {
        rec->state_version = version;
    }
}

void SwarmCore::handle_field_tick(const PacketHeader& h, const uint8_t* buf, size_t len) {
    FieldTickPayload p = {};
    PacketHeader decoded;
    if (!decode_field_tick(buf, len, decoded, p)) {
        return;
    }
    if (p.membership_epoch < rt_.membership.epoch && rt_.membership.count > 1) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, millis());
    if (p.coordinator_id != 0 && p.coordinator_id != rt_.self.node_id) {
        const uint32_t now_ms = millis();
        if (join_should_adopt(rt_.self.node_id,
                              rt_.membership.count,
                              rt_.membership.epoch,
                              rt_.coordinator,
                              discovering(now_ms),
                              p.coordinator_id,
                              2,
                              p.membership_epoch)) {
            adopt_coordinator(p.coordinator_id, p.membership_epoch, now_ms);
        } else if (rt_.coordinator == 0 || p.coordinator_id == rt_.coordinator) {
            rt_.coordinator = p.coordinator_id;
            rt_.membership.coordinator = p.coordinator_id;
            rt_.coordinator_is_self = false;
            rt_.last_coord_seen_ms = now_ms;
        }
    }
    uint64_t before = clock_.tick;
    tick_clock_set(clock_, p.tick, p.dt);
    rt_.tick = clock_.tick;
    last_field_ms_ = millis();
    if (clock_.tick > before) {
        advance_field();
    }
}

void SwarmCore::handle_command(const PacketHeader& h, const uint8_t* buf, size_t len) {
    CommandPayload p = {};
    PacketHeader decoded;
    if (!decode_command(buf, len, decoded, p)) {
        return;
    }
    note_peer(h.sender_id, h.boot_id, 0, h.sequence, millis());
    if (p.target_node != 0 && p.target_node != rt_.self.node_id) {
        return;
    }
    switch (p.opcode) {
        case CMD_INJECT: {
            uint8_t ch = static_cast<uint8_t>(p.arg0);
            if (ch < CH_COUNT) {
                inject(static_cast<Channel>(ch), p.arg1);
            }
            break;
        }
        case CMD_ELECT:
            request_election(millis());
            break;
        case CMD_RESET:
            say_goodbye();
            break;
        default:
            break;
    }
}

void SwarmCore::handle_goodbye(const PacketHeader& h) {
    NeighborRecord* rec = neighbor_find(rt_.neighbors, h.sender_id);
    if (rec != nullptr) {
        rec->alive = false;
    }
    NeighborTable compact;
    neighbor_table_clear(compact);
    for (uint8_t i = 0; i < rt_.neighbors.count; ++i) {
        if (rt_.neighbors.rows[i].node_id == h.sender_id) {
            continue;
        }
        compact.rows[compact.count++] = rt_.neighbors.rows[i];
    }
    rt_.neighbors = compact;
    membership_remove(rt_.membership, h.sender_id);
    field_store_drop_neighbor(field_, h.sender_id);
    rt_.membership.epoch++;
    if (h.sender_id == rt_.coordinator) {
        rt_.coordinator = 0;
        rt_.membership.coordinator = 0;
        rt_.coordinator_is_self = false;
    } else if (rt_.coordinator_is_self) {
        send_membership();
    }
}

#endif
