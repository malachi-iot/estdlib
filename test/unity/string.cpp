#include "unit-test.h"

#include <estd/string.h>
#include <estd/charconv.h>

#define TEST_STR "hi2u"
#define TEST_STR2 "hi2me"

template <class TString>
void _test_string_assignment()
{
    TString s = TEST_STR;

    TEST_ASSERT_EQUAL(sizeof(TEST_STR) - 1, s.size());

    //typedef typename TString::value_type char_type;
    //typedef typename TString::pointer pointer_type;
    const char* data = s.data();

    // Don't fiddle with null termination since it may or may not be available
    // depending on flavor of TString
    TEST_ASSERT_EQUAL_INT8_ARRAY(TEST_STR, data, s.size());
}


template <class TString>
static void test_string_concat()
{
    TString s;

    s += TEST_STR;
    s += ':';
    s += TEST_STR2;

    const char* data = s.clock();

    TEST_ASSERT_EQUAL_STRING(TEST_STR ":" TEST_STR2, data);
}

void test_layer1_string()
{
    _test_string_assignment<estd::layer1::string<32, true> >();
    _test_string_assignment<estd::layer1::string<32, false> >();

    test_string_concat<estd::layer1::string<32, true> >();
}


void test_layer2_string()
{
    // FIX: Causes exception on line 27
    _test_string_assignment<estd::layer2::const_string>();
}


void test_layer3_string()
{
    // TODO: "No suitable constructor exists to convert const char*""
    //_test_string_assignment<estd::layer3::const_string>();
}


void test_from_chars()
{
    estd::layer2::const_string s = "1234";
    long value;

    estd::from_chars_result result = 
        estd::from_chars(s.data(), s.data() + s.size(), value);

    TEST_ASSERT_EQUAL_INT32(0, (int)result.ec);
    TEST_ASSERT_EQUAL_INT32(1234, value);
}

static void test_to_chars()
{
    estd::layer1::string<32> s;

    estd::to_chars_result result = estd::to_chars(s.data(), s.data() + s.max_size(), 771);

    TEST_ASSERT_EQUAL('7', s[0]);
    TEST_ASSERT_EQUAL('7', s[1]);
    TEST_ASSERT_EQUAL('1', s[2]);

    result = estd::to_chars_opt(s.data(), s.data() + s.max_size(), 0xF0, 16);

    TEST_ASSERT_EQUAL(result.ptr, s.data() + 30);
    TEST_ASSERT_EQUAL('f', s[30]);
    TEST_ASSERT_EQUAL('0', s[31]);
}

static void test_to_string_opt()
{
    char buffer[64];
    int val = 123;

    estd::to_chars_result r = estd::to_string_opt(buffer, val, 10);

    // only support ints at this time
    //estd::internal::to_string_opt(buffer, 3.4, 10);

    TEST_ASSERT_EQUAL_STRING("123", r.ptr);
}


static void test_to_string()
{
#ifdef FEATURE_CPP_DEFAULT_TARGS
    estd::layer1::string<64> buffer;
    int val1 = 123;
    buffer += estd::to_string(val1);

    TEST_ASSERT_EQUAL_STRING("123", buffer.data());
#endif
}


// NOTE: Depending on feature flags, estd::char_traits is either our implementation
// or an alias for std::char_traits
static void test_char_traits()
{
    typedef estd::char_traits<char> traits_type;

    TEST_ASSERT_EQUAL(sizeof(TEST_STR) - 1, traits_type::length(TEST_STR));

    TEST_ASSERT_EQUAL(0, traits_type::compare(TEST_STR, TEST_STR2, 3));
    TEST_ASSERT_GREATER_THAN(0, traits_type::compare(TEST_STR, TEST_STR2, 5));
}


#ifdef ESP_IDF_TESTING
TEST_CASE("string tests", "[string]")
#else
void test_string()
#endif
{
    RUN_TEST(test_layer1_string);
    RUN_TEST(test_layer2_string);
    RUN_TEST(test_from_chars);
    RUN_TEST(test_to_chars);
    RUN_TEST(test_to_string);
    RUN_TEST(test_to_string_opt);
    RUN_TEST(test_char_traits);
}