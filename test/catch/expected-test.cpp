#include <catch.hpp>

#include <estd/system_error.h>

#include <estd/expected.h>

TEST_CASE("expected")
{
    estd::expected<int, estd::errc> e(10);

    // TODO: errc's enum_class is stopping us here - I think it needs a default ctor too
    //estd::expected<void, estd::errc> e2;

    REQUIRE(*e == 10);
}