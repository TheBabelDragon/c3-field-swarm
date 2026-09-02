#include <unity.h>

#include "swarm/join.h"

void test_discovery_window() {
    TEST_ASSERT_TRUE(join_in_discovery(100, 2250));
    TEST_ASSERT_FALSE(join_in_discovery(2250, 2250));
    TEST_ASSERT_FALSE(join_in_discovery(3000, 2250));
    TEST_ASSERT_FALSE(join_in_discovery(50, 0));
}

void test_new_device_adopts_live_coordinator_during_discovery() {
    TEST_ASSERT_TRUE(join_should_adopt(1, 1, 1, 0, true, 5, 4, 12));
}

void test_solo_self_elected_still_yields_to_swarm() {
    // Fresh flash already elected itself (coord=1, epoch=2, 1 member).
    TEST_ASSERT_TRUE(join_should_adopt(1, 1, 2, 1, false, 5, 5, 9));
}

void test_established_coordinator_does_not_yield_to_rejoin() {
    TEST_ASSERT_FALSE(join_should_adopt(5, 5, 9, 5, false, 6, 1, 2));
}

void test_higher_epoch_wins_on_partition_merge() {
    TEST_ASSERT_TRUE(join_should_adopt(2, 3, 4, 2, false, 6, 3, 10));
}

void test_never_adopt_self_or_empty() {
    TEST_ASSERT_FALSE(join_should_adopt(3, 1, 1, 0, true, 3, 4, 8));
    TEST_ASSERT_FALSE(join_should_adopt(3, 1, 1, 0, true, 0, 4, 8));
}

void test_lower_hardware_id_keeps_logical_id() {
    TEST_ASSERT_TRUE(join_should_yield_node_id(0xBB, 0xAA));
    TEST_ASSERT_FALSE(join_should_yield_node_id(0xAA, 0xBB));
    TEST_ASSERT_FALSE(join_should_yield_node_id(0xAA, 0xAA));
}

void test_next_free_id_skips_taken() {
    NodeId taken[3] = {1, 2, 4};
    TEST_ASSERT_EQUAL_UINT32(3, join_next_free_id(taken, 3));
}

void test_next_free_id_full_swarm() {
    NodeId taken[6] = {1, 2, 3, 4, 5, 6};
    TEST_ASSERT_EQUAL_UINT32(0, join_next_free_id(taken, 6));
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_discovery_window);
    RUN_TEST(test_new_device_adopts_live_coordinator_during_discovery);
    RUN_TEST(test_solo_self_elected_still_yields_to_swarm);
    RUN_TEST(test_established_coordinator_does_not_yield_to_rejoin);
    RUN_TEST(test_higher_epoch_wins_on_partition_merge);
    RUN_TEST(test_never_adopt_self_or_empty);
    RUN_TEST(test_lower_hardware_id_keeps_logical_id);
    RUN_TEST(test_next_free_id_skips_taken);
    RUN_TEST(test_next_free_id_full_swarm);
    return UNITY_END();
}
