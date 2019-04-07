#include <catch.hpp>

#include <estd/chrono.h>
#include <estd/limits.h>

using namespace estd;

TEST_CASE("limits & common_type tests")
{
    // https://stackoverflow.com/questions/15211463/why-isnt-common-typelong-unsigned-longtype-long-long
    // summarizes for us that common_type will not go to something like an int64_t - instead it is going
    // to choose one of the two
    SECTION("common_type")
    {
        SECTION("signed test 1")
        {
            typedef typename common_type<int16_t, uint32_t>::type common_type;

            //int sz = sizeof(common_type);

            //REQUIRE(sz == 4);
            //REQUIRE((numeric_limits<common_type>::digits) == 32);
            //auto max = numeric_limits<common_type>::max();
            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be unsigned
            REQUIRE(!is_signed);
        }
        SECTION("signed test 2")
        {
            typedef typename common_type<uint16_t, int32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 31);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 3")
        {
            typedef typename common_type<int32_t, uint32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(!is_signed);
        }
    }
    SECTION("8 bit")
    {
        auto digits = numeric_limits<int8_t>::digits;

        REQUIRE(digits == 7);

        digits = numeric_limits<uint8_t>::digits;

        REQUIRE(digits == 8);
    }
    SECTION("32 bit")
    {
        auto digits = numeric_limits<int32_t>::digits;

        REQUIRE(digits == 31);

        digits = numeric_limits<uint32_t>::digits;

        REQUIRE(digits == 32);
    }
}
