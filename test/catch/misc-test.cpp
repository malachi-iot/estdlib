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
        using it = typename type::const_local_iterator;

        type map;

        // FIX: Key '0' not working due to collision between hash(0) and Null

        map.insert({1, "hi2u"});
        int bucket1 = map.bucket(1);
        unsigned counter = 0;

        // NOTE: Undefined behavior, but valid in this tightly controlled unit test
        REQUIRE(bucket1 == 1);

        for(it i = map.cbegin(bucket1); i != map.end(bucket1); ++i, ++counter)
        {

        }

        REQUIRE(counter == 1);
    }
}
