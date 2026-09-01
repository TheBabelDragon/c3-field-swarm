#include <unity.h>
#include <math.h>

#include "field/field_state.h"
#include "field/field_delta.h"
#include "field/field_view.h"
#include "field/field_system.h"
#include "systems/diffusion.h"
#include "systems/information_decay.h"
#include "systems/aggregation.h"

void test_channel_get_set() {
    FieldState s;
    field_state_zero(s);
    field_channel_set(s, CH_SIGNAL, 0.5f);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, field_channel_get(s, CH_SIGNAL));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, field_channel_get(s, CH_TEMPERATURE));
}

void test_delta_sort_is_stable_key() {
    FieldDeltaList list;
    field_delta_list_clear(list);
    FieldDelta a{3, 10, CH_SIGNAL, 0, 1};
    FieldDelta b{1, 10, CH_SIGNAL, 0, 2};
    FieldDelta c{1, 9, CH_TEMPERATURE, 0, 3};
    field_delta_list_push(list, a);
    field_delta_list_push(list, b);
    field_delta_list_push(list, c);
    field_delta_sort(list);
    TEST_ASSERT_EQUAL_UINT64(9, list.items[0].tick);
    TEST_ASSERT_EQUAL_UINT32(1, list.items[1].source_node);
    TEST_ASSERT_EQUAL_UINT32(3, list.items[2].source_node);
}

void test_diffusion_formula() {
    FieldStore store;
    field_store_init(store);
    store.local.signal = 0.0f;
    FieldState n1;
    field_state_zero(n1);
    n1.signal = 1.0f;
    field_store_set_neighbor(store, 2, n1, 1);
    FieldView view;
    field_view_freeze(view, store, 1, 1, 0.5f);
    DiffusionSystem diff(0.25f);
    FieldDeltaList out;
    field_delta_list_clear(out);
    diff.evaluate(view, out);
    TEST_ASSERT_TRUE(out.count >= 1);
    bool found = false;
    for (uint8_t i = 0; i < out.count; ++i) {
        if (out.items[i].channel == CH_SIGNAL) {
            TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.25f, out.items[i].new_value);
            found = true;
        }
    }
    TEST_ASSERT_TRUE(found);
}

void test_decay_formula() {
    FieldStore store;
    field_store_init(store);
    store.local.information = 1.0f;
    FieldView view;
    field_view_freeze(view, store, 3, 4, 0.5f);
    InformationDecaySystem decay(0.08f);
    FieldDeltaList out;
    field_delta_list_clear(out);
    decay.evaluate(view, out);
    TEST_ASSERT_EQUAL_UINT8(1, out.count);
    float expect = expf(-0.08f * 0.5f);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, expect, out.items[0].new_value);
}

void test_systems_do_not_write_view() {
    FieldStore store;
    field_store_init(store);
    store.local.information = 0.77f;
    FieldView view;
    field_view_freeze(view, store, 1, 1, 0.5f);
    InformationDecaySystem decay;
    FieldDeltaList out;
    field_delta_list_clear(out);
    decay.evaluate(view, out);
    TEST_ASSERT_EQUAL_FLOAT(0.77f, view.local.information);
}

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_channel_get_set);
    RUN_TEST(test_delta_sort_is_stable_key);
    RUN_TEST(test_diffusion_formula);
    RUN_TEST(test_decay_formula);
    RUN_TEST(test_systems_do_not_write_view);
    return UNITY_END();
}
