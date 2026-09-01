#include <unity.h>
#include <string.h>

#include "field/field_state.h"
#include "field/field_view.h"
#include "field/field_system.h"
#include "systems/diffusion.h"
#include "systems/information_decay.h"
#include "systems/aggregation.h"

static FieldState run_once(const FieldState& initial,
                           const FieldState* neighbors,
                           const NodeId* neighbor_ids,
                           uint8_t n_neighbors,
                           uint64_t tick,
                           float dt) {
    FieldStore store;
    field_store_init(store);
    store.local = initial;
    for (uint8_t i = 0; i < n_neighbors; ++i) {
        field_store_set_neighbor(store, neighbor_ids[i], neighbors[i], 1);
    }
    FieldView view;
    field_view_freeze(view, store, 3, tick, dt);
    DiffusionSystem diffusion(0.25f);
    InformationDecaySystem decay(0.08f);
    AggregationSystem agg;
    FieldSystem* systems[3] = {&decay, &diffusion, &agg};
    FieldDeltaList applied;
    FieldState next = store.local;
    field_run_systems(view, systems, 3, next, applied);
    return next;
}

void test_same_inputs_same_outputs() {
    FieldState initial;
    field_state_zero(initial);
    initial.information = 0.77f;
    initial.signal = 0.36f;
    initial.temperature = 0.42f;
    initial.energy = 0.11f;
    FieldState nb[2];
    field_state_zero(nb[0]);
    field_state_zero(nb[1]);
    nb[0].information = 0.10f;
    nb[0].signal = 1.00f;
    nb[1].information = 0.20f;
    nb[1].signal = 0.00f;
    NodeId ids[2] = {2, 4};
    FieldState a = run_once(initial, nb, ids, 2, 18421, 0.5f);
    FieldState b = run_once(initial, nb, ids, 2, 18421, 0.5f);
    TEST_ASSERT_EQUAL_FLOAT(a.temperature, b.temperature);
    TEST_ASSERT_EQUAL_FLOAT(a.information, b.information);
    TEST_ASSERT_EQUAL_FLOAT(a.energy, b.energy);
    TEST_ASSERT_EQUAL_FLOAT(a.signal, b.signal);
}

void test_neighbor_insert_order_does_not_matter() {
    FieldState initial;
    field_state_zero(initial);
    initial.signal = 0.5f;
    initial.information = 1.0f;
    FieldState n_low;
    FieldState n_high;
    field_state_zero(n_low);
    field_state_zero(n_high);
    n_low.signal = 0.0f;
    n_low.information = 0.0f;
    n_high.signal = 1.0f;
    n_high.information = 0.5f;
    FieldState first_order_neighbors[2] = {n_high, n_low};
    NodeId first_ids[2] = {6, 1};
    FieldState second_order_neighbors[2] = {n_low, n_high};
    NodeId second_ids[2] = {1, 6};
    FieldState a = run_once(initial, first_order_neighbors, first_ids, 2, 10, 0.5f);
    FieldState b = run_once(initial, second_order_neighbors, second_ids, 2, 10, 0.5f);
    TEST_ASSERT_EQUAL_FLOAT(a.temperature, b.temperature);
    TEST_ASSERT_EQUAL_FLOAT(a.information, b.information);
    TEST_ASSERT_EQUAL_FLOAT(a.energy, b.energy);
    TEST_ASSERT_EQUAL_FLOAT(a.signal, b.signal);
}

void test_system_registration_order_decay_then_diffusion_is_canonical() {
    FieldState initial;
    field_state_zero(initial);
    initial.information = 1.0f;
    FieldState nb;
    field_state_zero(nb);
    nb.information = 0.0f;
    NodeId id = 2;
    FieldState out = run_once(initial, &nb, &id, 1, 1, 0.5f);
    TEST_ASSERT_TRUE(out.information < 1.0f);
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_same_inputs_same_outputs);
    RUN_TEST(test_neighbor_insert_order_does_not_matter);
    RUN_TEST(test_system_registration_order_decay_then_diffusion_is_canonical);
    return UNITY_END();
}
