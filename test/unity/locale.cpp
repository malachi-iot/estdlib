#include "unit-test.h"

#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>


using namespace estd;

static void test_use_facet()
{
    const char* truename = 
        use_facet<numpunct<char>>(locale::classic()).truename().data();

#if UNITY_VERSION < 0x200
    TEST_ASSERT_EQUAL('t', truename[0]);
#else
    TEST_ASSERT_EQUAL_STRING("true", truename);
#endif
}

static void test_num_get()
{
    const char* input = "123 456/789";
    typedef experimental::istringstream<64> streambuf_type;
    streambuf_type rdbuf(input);
    typedef experimental::istreambuf_iterator<streambuf_type> iterator_type;

    ios_base::iostate err;
    unsigned val;

    auto facet = use_facet<num_get<char, iterator_type> > (locale::classic());

    // FIX: Dies trying compile sgetc call in iterator
#if NOTREADY
    iterator_type it(rdbuf), end;

    it = facet.get(it, end, rdbuf, err, val);

    TEST_ASSERT_EQUAL(123, val);
#endif
}


#ifdef ESP_IDF_TESTING
TEST_CASE("estd::locale and friends", "[locale]")
#else
void test_locale()
#endif
{
    RUN_TEST(test_use_facet);
    RUN_TEST(test_num_get);
}