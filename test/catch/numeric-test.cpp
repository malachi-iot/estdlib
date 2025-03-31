#include <catch2/catch_all.hpp>

#include <estd/array.h>
#include <estd/numeric.h>

using namespace estd;

TEST_CASE("numeric")
{
    // DEBT: Can we do a CTAD here?
    const array<int, 4> things { 1, 2, 3, 4 };

    SECTION("accumulate")
    {
        int v = accumulate(things.begin(), things.end(), 0);

        REQUIRE(v == 10);

        v = accumulate(things.begin(), things.end(), 0, [](int lhs, int rhs)
        {
            return lhs + rhs * 2;
        });

        REQUIRE(v == 20);
    }
}