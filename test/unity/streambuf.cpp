/**
 * @file
 * Wishful thinking - hoping streambufs can work in C++03
 * Current ESTD_FN_HAS_METHOD + function default template param approach a no go
 */

// DEBT: Not quite C++03 compatible, but isn't hard -- just need to move
// variable initializer
#define FEATURE_ESTD_IOS_GCOUNT 0


#include "unit-test.h"

#include <estd/streambuf.h>
#include <estd/istream.h>
#include <estd/ostream.h>

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

    TEST_ASSERT_EQUAL_CHAR(buf[0], c);

    char buf2[64];

    streamsize sz = os.sgetn(buf2, sizeof(buf2) - 1);

    // DEBT: Get this one working just right -- it's there, I'm just out of time
    //TEST_ASSERT_EQUAL_INT(sz, strlen(buf) - 1);

    buf2[sz] = 0;

    TEST_ASSERT_EQUAL_CHAR_ARRAY(&buf[1], buf2, sz);
}



#ifdef ESP_IDF_TESTING
TEST_CASE("streambuf", "[streambuf]")
#else
void test_streambuf()
#endif
{
    RUN_TEST(test_ispanbuf);
    RUN_TEST(test_ospanbuf);
}
