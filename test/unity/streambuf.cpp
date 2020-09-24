/**
 * @file
 * Wishful thinking - hoping streambufs can work in C++03
 * Current ESTD_FN_HAS_METHOD + function default template param approach a no go
 */

#include "unit-test.h"

#include <estd/streambuf.h>
#include <estd/istream.h>
#include <estd/ostream.h>

static void test_ospanbuf()
{
    char buf[128];
    estd::span<char> span(buf);

    estd::experimental::ospanbuf os(span);
}


static void test_ispanbuf()
{
    char buf[] = "hi2u 1234";
    estd::span<char> span(buf);
    
    estd::experimental::ispanbuf os(span);
}



#ifdef ESP_IDF_TESTING
TEST_CASE("streambuf", "[streambuf]")
#else
void test_streambuf()
#endif
{
    RUN_TEST(test_ospanbuf);
}
