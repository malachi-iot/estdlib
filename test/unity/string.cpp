#include "unit-test.h"

#include <estd/string.h>

#define TEST_STR "hi2u"
#define TEST_STR2 "hi2me"

template <class TString>
void _test_string_assignment()
{
    const char* test_str = TEST_STR;

    TString s = test_str;

    TEST_ASSERT_EQUAL(sizeof(TEST_STR) - 1, s.size());

    //typedef typename TString::value_type char_type;
    //typedef typename TString::pointer pointer_type;
    // NOTE: Only doing this in the safe confines of our unit test, knowing that
    // underlying strings passed into this test can in fact be safely modified
    // (needed for const_string flavor)
    char* data = const_cast<char*>(s.data());

    //if(!estd::is_const<char_type>::value)
    {
        // NOTE: Brute forcing, so that further asserts work:
        // a) null-terminated already has this
        // b) non-null-terminated must have space for this
        data[s.size()] = 0;
    }

    TEST_ASSERT_EQUAL_STRING(test_str, data);
}


template <class TString>
void test_string_concat()
{
    const char* test_str = "hi2u";
    const char* test_str2 = "hi2me";

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
    // FIX: non-null-terminated has an issue
    _test_string_assignment<estd::layer1::string<32, false> >();

    test_string_concat<estd::layer1::string<32, true> >();
}


void test_layer2_string()
{
    _test_string_assignment<estd::layer2::const_string>();
}


void test_layer3_string()
{
    // TODO: "No suitable constructor exists to convert const char*""
    //_test_string_assignment<estd::layer3::const_string>();
}


void test_string()
{
    RUN_TEST(test_layer1_string);
    RUN_TEST(test_layer2_string);
}