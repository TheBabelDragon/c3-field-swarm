#include <unity.h>

#include "swarm/election.h"
#include "swarm/neighbor.h"
#include "swarm/state.h"

void test_highest_node_id_wins() {
    NodeId ids[3] = {1, 5, 3};
    BootId boots[3] = {10, 2, 99};
    uint32_t caps[3] = {CAP_COORD_ELIGIBLE, CAP_COORD_ELIGIBLE, CAP_COORD_ELIGIBLE};
    ElectionResult r = elect_highest_eligible(ids, boots, caps, 3);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_EQUAL_UINT32(5, r.winner);
}

void test_tie_break_boot_id() {
    NodeId ids[2] = {4, 4};
    BootId boots[2] = {7, 11};
    uint32_t caps[2] = {CAP_COORD_ELIGIBLE, CAP_COORD_ELIGIBLE};
    ElectionResult r = elect_highest_eligible(ids, boots, caps, 2);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_EQUAL_UINT32(4, r.winner);
    TEST_ASSERT_EQUAL_UINT32(11, r.winner_boot);
}

void test_ineligible_skipped() {
    NodeId ids[3] = {6, 2, 5};
    BootId boots[3] = {1, 1, 1};
    uint32_t caps[3] = {CAP_NONE, CAP_COORD_ELIGIBLE, CAP_COORD_ELIGIBLE};
    ElectionResult r = elect_highest_eligible(ids, boots, caps, 3);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_EQUAL_UINT32(5, r.winner);
}

void test_rejoin_does_not_force_winner_change() {
    Membership m;
    membership_clear(m);
    membership_add(m, 2);
    membership_add(m, 3);
    membership_add(m, 4);
    m.coordinator = 4;
    m.epoch = 17;
    TEST_ASSERT_EQUAL_UINT32(4, m.coordinator);
    membership_add(m, 1);
    TEST_ASSERT_EQUAL_UINT32(4, m.coordinator);
    TEST_ASSERT_TRUE(membership_contains(m, 1));
}

void test_loss_of_coordinator_selects_remaining_highest() {
    NeighborTable remaining;
    neighbor_table_clear(remaining);
    NeighborRecord* r2 = neighbor_upsert(remaining, 2);
    NeighborRecord* r4 = neighbor_upsert(remaining, 4);
    r2->boot_id = 1;
    r2->capabilities = CAP_COORD_ELIGIBLE;
    r4->boot_id = 1;
    r4->capabilities = CAP_COORD_ELIGIBLE;
    NodeIdentity live;
    live.node_id = 3;
    live.boot_id = 8;
    live.hardware_id = 9;
    Membership live_m;
    membership_clear(live_m);
    membership_add(live_m, 2);
    membership_add(live_m, 3);
    membership_add(live_m, 4);
    ElectionResult r = elect_from_membership(live_m, remaining, live, CAP_COORD_ELIGIBLE);
    TEST_ASSERT_TRUE(r.valid);
    TEST_ASSERT_EQUAL_UINT32(4, r.winner);
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_highest_node_id_wins);
    RUN_TEST(test_tie_break_boot_id);
    RUN_TEST(test_ineligible_skipped);
    RUN_TEST(test_rejoin_does_not_force_winner_change);
    RUN_TEST(test_loss_of_coordinator_selects_remaining_highest);
    return UNITY_END();
}
