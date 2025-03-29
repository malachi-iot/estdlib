#include <catch2/catch_all.hpp>

#include <estd/array.h>
#include <estd/functional.h>
#include <estd/type_traits.h>
#include <estd/variant.h>
#include <estd/internal/container/set.h>
#include <estd/internal/container/unordered_set.h>
#include <estd/internal/container/unordered_map.h>

using namespace estd;

// For one-off tests not warranting its own test file
TEST_CASE("miscellaneous")
{
    SECTION("monostate")
    {
        REQUIRE(estd::is_empty_f<estd::monostate>());
        REQUIRE(monostate{} == monostate{});

        SECTION("hash")
        {
            estd::hash<monostate> hash_fn;

            REQUIRE(hash_fn(monostate{}) == hash_fn(monostate{}));
        }
    }
    SECTION("unordered_set")
    {
        using type = estd::internal::unordered_set<array<int, 10 > >;
        using rtype = pair<type::iterator, bool>;

        type value;

        REQUIRE(value.size() == 0);
        REQUIRE(value.empty());

        rtype r = value.insert(5);

        REQUIRE(r.second);

        REQUIRE(value.size() == 1);
        REQUIRE(value.empty() == false);

        r = value.insert(6);

        REQUIRE(r.second);

        REQUIRE(value.size() == 2);
    }
    SECTION("unordered_map")
    {
        using type = estd::internal::unordered_map<10, int, const char*>;
        using value_type = typename type::value_type;

        type map;

        map.insert({0, "hi2u"});
    }
}
