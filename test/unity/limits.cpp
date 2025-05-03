#include "unit-test.h"

#include <estd/limits.h>

#include <float.h>

#undef min
#undef max

static void test_limits_1()
{
    TEST_ASSERT(estd::numeric_limits<int8_t>::is_signed);
    TEST_ASSERT_EQUAL(127, estd::numeric_limits<int8_t>::max());
    TEST_ASSERT_EQUAL(19, estd::numeric_limits<uint64_t>::digits10);
    TEST_ASSERT_TRUE(INT64_MAX == estd::numeric_limits<int64_t>::max());
    TEST_ASSERT_EQUAL(LLONG_WIDTH, sizeof(long long) * 8);
}

#ifndef INT64_MAX
#error
#endif

#ifndef LONG_MAX
#error
#endif


/*
#ifndef LLONG_MAX
#error
#endif
*/

static void test_limits_least32()
{
    TEST_ASSERT(estd::numeric_limits<estd::uint_least32_t>::is_signed == false);
    TEST_ASSERT_EQUAL(UINT_LEAST32_MAX, estd::numeric_limits<uint_least32_t>::max());

    TEST_ASSERT(estd::numeric_limits<estd::int_least32_t>::is_signed);
    TEST_ASSERT_EQUAL(INT_LEAST32_MAX, estd::numeric_limits<int_least32_t>::max());
}


static void test_limits_least64()
{
#ifdef LLONG_WIDTH
    TEST_ASSERT(estd::numeric_limits<estd::uint_least64_t>::is_signed == false);
    TEST_ASSERT(estd::numeric_limits<estd::int_least64_t>::is_signed);
    TEST_ASSERT_GREATER_OR_EQUAL(8, sizeof(estd::uint_least64_t));
    TEST_ASSERT_GREATER_OR_EQUAL(8, sizeof(estd::int_least64_t));
    TEST_ASSERT_EQUAL(19, estd::numeric_limits<uint_least64_t>::digits10);

#if !UNITY_SUPPORT_64
    TEST_ASSERT_TRUE(UINT_LEAST64_MAX == estd::numeric_limits<uint_least64_t>::max());
    TEST_ASSERT_TRUE(INT_LEAST64_MAX == estd::numeric_limits<int_least64_t>::max());
#else
    TEST_ASSERT_EQUAL_UINT64(UINT_LEAST64_MAX, estd::numeric_limits<uint_least64_t>::max());
    TEST_ASSERT_EQUAL_INT64(INT_LEAST64_MAX, estd::numeric_limits<int_least64_t>::max());
#endif
#endif
}

#if FEATURE_ESTD_COMPILE_VERBOSITY > ESTD_LEVEL_MEDIUM
#if defined(__GCC_IEC_559)
#pragma message "__GCC_IEC_559 defined"
#if __GCC_IEC_559 == 0
#pragma message "__GCC_IEC_559 == 0"
#endif
#endif
#if defined(__STDC_IEC_60559_BFP__)
#pragma message "__STDC_IEC_559__ defined"
#if __STDC_IEC_60559_BFP__ == 0
#pragma message "__STDC_IEC_559__ == 0"
#endif
#endif
#if defined(__STDC_IEC_60559_BFP__)
#pragma message "__STDC_IEC_60559_BFP__ defined"
#if __STDC_IEC_60559_BFP__ == 0
#pragma message "__STDC_IEC_60559_BFP__ == 0"
#endif
#endif
#endif


static void test_limits_float()
{
#if __GCC_IEC_559 || __STDC_IEC_559__ || __STDC_IEC_60559_BFP__
    using type = estd::numeric_limits<float>;

    TEST_ASSERT_TRUE(type::is_iec559);
#elif FEATURE_ESTD_COMPILE_VERBOSITY >= ESTD_LEVEL_MEDIUM
#warning Skipping IEC_559 test
#endif
}


static void test_limits_double()
{
#if __GCC_IEC_559 || __STDC_IEC_559__ || __STDC_IEC_60559_BFP__
    using type = estd::numeric_limits<double>;

    TEST_ASSERT_TRUE(type::is_iec559);
#endif
}


#ifdef ESP_IDF_TESTING
TEST_CASE("limits tests", "[limits]")
#else
void test_limits()
#endif
{
    RUN_TEST(test_limits_1);
    RUN_TEST(test_limits_least32);
    RUN_TEST(test_limits_least64);
    RUN_TEST(test_limits_float);
    RUN_TEST(test_limits_double);
}
