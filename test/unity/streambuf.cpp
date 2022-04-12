/**
 * @file
 * Wishful thinking - hoping streambufs can work in C++03
 * Current ESTD_FN_HAS_METHOD + function default template param approach a no go
 */

#include "unit-test.h"

#include <estd/streambuf.h>
#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

using namespace estd;

static void test_ospanbuf()
{
    char buf[128];
    estd::span<char> span(buf);

    estd::experimental::ospanbuf os(span);

    os.sputc('a');
    os.sputn(" test 567", 9);

    TEST_ASSERT_EQUAL_INT(10, os.pos());
    TEST_ASSERT_EQUAL_INT(10, os.pubseekoff(0, ios_base::cur));
}


static void test_ispanbuf()
{
    char buf[] = "hi2u 1234";
    estd::span<char> span(buf);
    
    estd::experimental::ispanbuf os(span);

    char c = os.sbumpc();

#ifdef ESP_IDF_TESTING
    TEST_ASSERT_EQUAL(buf[0], c);
#else
    TEST_ASSERT_EQUAL_CHAR(buf[0], c);
#endif

    char buf2[64];

    streamsize sz = os.sgetn(buf2, sizeof(buf2) - 1);

    // DEBT: Get this one working just right -- it's there, I'm just out of time
    //TEST_ASSERT_EQUAL_INT(sz, strlen(buf) - 1);

    buf2[sz] = 0;

    // TODO: See what we can do about this in esp-idf
#ifndef ESP_IDF_TESTING
    TEST_ASSERT_EQUAL_CHAR_ARRAY(&buf[1], buf2, sz);
#endif
}

static void test_ospanstream()
{
    char buf[128];
    estd::span<char> span(buf);

    estd::experimental::ospanstream os(span);

    os << "hi2u";

    TEST_ASSERT_EQUAL_INT(4, os.tellp());
    TEST_ASSERT_EQUAL('h', *os.rdbuf()->pbase());
}


// DEBT: Clumsy
#include <estd/internal/istream_runtimearray.hpp>


static void test_ispanstream()
{
    char buf[] = "hi2u 1234";
    // DEBT: make const char span operable
    estd::span<char> span(buf);

    estd::experimental::ispanstream is(span);

    estd::layer1::string<64> s;

    // Only extraction operator supported at this time is for string
    is >> s;

    TEST_ASSERT_EQUAL_INT(4, s.length());
    TEST_ASSERT_EQUAL('h', s[0]);

    is >> s;

    TEST_ASSERT_EQUAL_INT(4, s.length());
    TEST_ASSERT_EQUAL('1', s[0]);
}



#ifdef ESP_IDF_TESTING
TEST_CASE("streambuf", "[streambuf]")
#else
void test_streambuf()
#endif
{
    RUN_TEST(test_ispanbuf);
    RUN_TEST(test_ospanbuf);
    RUN_TEST(test_ispanstream);
    RUN_TEST(test_ospanstream);
}
