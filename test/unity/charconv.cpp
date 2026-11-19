#include "unit-test.h"

#include <estd/charconv.h>
#include <estd/string.h>

using namespace estd;

static void test_from_chars()
{
    constexpr layer2::const_string s = "1234";
    long value;

    from_chars_result result = from_chars(s.data(), s.data() + s.size(), value);

    TEST_ASSERT_EQUAL_INT32(0, (int)result.ec);
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

static void test_to_chars()
{
    layer1::string<32> s;
    const uint64_t biguint = 999999999999999;
    const int64_t bigint = 999999999999999;

    to_chars_result result = estd::to_chars(s.data(), s.data() + s.max_size(), 771);

    TEST_ASSERT_EQUAL('7', s[0]);
    TEST_ASSERT_EQUAL('7', s[1]);
    TEST_ASSERT_EQUAL('1', s[2]);

    result = to_chars_opt(s.data(), s.data() + s.max_size(), 0xF0, 16);

    // max_size() is going to be 31, it's a null-terminated string
    
    TEST_ASSERT_EQUAL(result.ptr, s.data() + 29);
    TEST_ASSERT_EQUAL('f', s[29]);
    TEST_ASSERT_EQUAL('0', s[30]);

    result = to_chars(s.data(), s.data() + s.max_size(), bigint);

    TEST_ASSERT_EQUAL('9', s[0]);
    TEST_ASSERT_EQUAL('9', s[1]);
    TEST_ASSERT_EQUAL('9', s[2]);
    TEST_ASSERT_EQUAL('9', s[10]);

    result = to_chars(s.data(), s.data() + s.max_size(), biguint);

    TEST_ASSERT_EQUAL('9', s[0]);
    TEST_ASSERT_EQUAL('9', s[1]);
    TEST_ASSERT_EQUAL('9', s[2]);
    TEST_ASSERT_EQUAL('9', s[10]);
}


static void test_to_chars_detail()
{
    char out[32]{};

    to_chars_result r = detail::to_chars_upper<16>(out, out + 2, 0xAF);

    TEST_ASSERT_EQUAL(errc{}, r.ec);
    TEST_ASSERT_EQUAL_STRING("AF", out);
}


#ifdef ESP_IDF_TESTING
TEST_CASE("string tests", "[string]")
#else
void test_charconv()
#endif
{
    RUN_TEST(test_from_chars);
    RUN_TEST(test_to_chars);
    RUN_TEST(test_to_chars_detail);
}