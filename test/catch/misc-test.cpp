#include <catch2/catch_all.hpp>

#include <estd/array.h>
#include <estd/functional.h>
#include <estd/numbers.h>
#include <estd/type_traits.h>
#include <estd/string.h>
#include <estd/variant.h>
#include <estd/internal/container/set.h>

using namespace estd;

// For one-off tests not warranting its own test file
TEST_CASE("miscellaneous")
{
    SECTION("monostate")
    {
        REQUIRE(estd::is_empty<estd::monostate>());
        REQUIRE(monostate{} == monostate{});

        SECTION("hash")
        {
            estd::hash<monostate> hash_fn;

            REQUIRE(hash_fn(monostate{}) == hash_fn(monostate{}));
        }
    }
    SECTION("numbers")
    {
        auto e = numbers::e_v<float>;

        // DEBT: We expect 'float' precision to generally operate the same across compilers, but IIRC that is
        // not a gauruntee
        REQUIRE(e == 2.718281746f);
    }
}
