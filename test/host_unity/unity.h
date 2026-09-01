#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

static int unity_failures = 0;
static int unity_tests = 0;

#define TEST_ASSERT_TRUE(cond) do { \
    if (!(cond)) { printf("FAIL %s:%d TRUE\n", __FILE__, __LINE__); ++unity_failures; } \
} while (0)

#define TEST_ASSERT_FALSE(cond) TEST_ASSERT_TRUE(!(cond))

#define TEST_ASSERT_EQUAL_UINT8(exp, got) TEST_ASSERT_TRUE((exp) == (got))
#define TEST_ASSERT_EQUAL_UINT32(exp, got) TEST_ASSERT_TRUE((uint32_t)(exp) == (uint32_t)(got))
#define TEST_ASSERT_EQUAL_UINT64(exp, got) TEST_ASSERT_TRUE((uint64_t)(exp) == (uint64_t)(got))
#define TEST_ASSERT_EQUAL_FLOAT(exp, got) TEST_ASSERT_TRUE(fabsf((float)(exp) - (float)(got)) < 1e-5f)
#define TEST_ASSERT_FLOAT_WITHIN(delta, exp, got) TEST_ASSERT_TRUE(fabsf((float)(exp) - (float)(got)) <= (float)(delta))

#define UNITY_BEGIN() (unity_failures = 0, unity_tests = 0, 0)
#define RUN_TEST(fn) do { ++unity_tests; printf("- %s\n", #fn); fn(); } while (0)
#define UNITY_END() (printf("%d tests, %d failures\n", unity_tests, unity_failures), unity_failures)
