#include "unit-test.h"

#include <estd/istream.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>


using namespace estd;

static void test_use_facet()
{
    const char* truename = 
        use_facet<numpunct<char> >(locale::classic()).truename().data();

#if UNITY_VERSION < 0x200
    TEST_ASSERT_EQUAL('t', truename[0]);
#else
    TEST_ASSERT_EQUAL_STRING("true", truename);
#endif
}

static const char input[] = "123 456/789";

template <class TNumGet, class TStr>
static void _test_num_get(TNumGet& facet, 
    TStr& istream,
    typename TNumGet::iter_type it,
    typename TNumGet::iter_type end)
{
    ios_base::iostate err = ios_base::goodbit;
    unsigned val = 0;

    it = facet.get(it, end, istream, err, val);

    TEST_ASSERT_EQUAL(123, val);
    TEST_ASSERT_EQUAL(' ', *it++);

    // DEBT: Manually skip the space, because istreambuf_iterator needs work
    // before >> ws works
    //++it;
    // Not doing that yet, because we found a parsing glitch so dealing with that first

    it = facet.get(it, end, istream, err, val);

    TEST_ASSERT_EQUAL(ios_base::goodbit, err);
    TEST_ASSERT_EQUAL(456, val);

    it = facet.get(it, end, istream, err, val);

    // '/' makes this fail
    TEST_ASSERT_EQUAL(ios_base::failbit, err);
    TEST_ASSERT_EQUAL(0, val);
}

static void test_num_get()
{
    experimental::istringstream<64> istream;   // Just for locale
    num_get<char, const char*> facet = use_facet<num_get<char, const char*> >(istream.getloc());

    _test_num_get(facet, istream, input, input + sizeof(input) - 1);
}

static void test_num_get_istream1()
{
    typedef experimental::istringstream<64> istream_type;
    typedef typename istream_type::streambuf_type streambuf_type;
    istream_type istream(input);
    typedef istreambuf_iterator<streambuf_type> iterator_type;

    num_get<char, iterator_type> facet = use_facet<num_get<char, iterator_type> >(istream.getloc());

    iterator_type it(istream), end;

    _test_num_get(facet, istream, it, end);
}


static void test_num_get_istream2()
{
    typedef experimental::istringstream<64> istream_type;
    typedef typename istream_type::streambuf_type streambuf_type;
    istream_type istream(input);

#ifdef FEATURE_CPP_AUTO
    auto facet = use_facet<num_get<streambuf_type> >(istream.getloc());

    _test_num_get(facet, istream, *istream.rdbuf(), {});
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
    RUN_TEST(test_num_get_istream1);
    RUN_TEST(test_num_get_istream2);
}