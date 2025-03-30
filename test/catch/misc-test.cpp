#include <catch2/catch_all.hpp>

#include <estd/array.h>
#include <estd/functional.h>
#include <estd/type_traits.h>
#include <estd/string.h>
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
        using type = estd::internal::unordered_map<10, int, layer1::string<32>>;
        //using value_type = typename type::value_type;
        using iter = typename type::local_iterator;
        using const_iter = typename type::const_local_iterator ;
        using pair = estd::pair<typename type::iterator, bool>;

        type map;
        

        // FIX: Linear probing may span across two (or more) different
        // key spaces before it finds something, resulting in incorrect and undiscoverable
        // placements.  We need to check against bucket a bit more during insert/emplace.
        // FIX: Key '0' not working due to collision between hash(0) and Null

        pair r1 = map.insert({1, "hi2u"});
        REQUIRE(r1.second);
        r1 = map.insert({1, "hi again"});
        REQUIRE(r1.second == false);
        REQUIRE(r1.first->second == "hi2u");
        int bucket1 = map.bucket(1);
        unsigned counter = 0;

        // NOTE: Undefined behavior, but valid in this tightly controlled unit test
        REQUIRE(bucket1 == type::bucket_depth);     // key 1 x bucket_depth

        for(const_iter i = map.cbegin(bucket1); i != map.end(bucket1); ++i, ++counter)
        {
            REQUIRE(i->second == "hi2u");
        }

        REQUIRE(counter == 1);

        REQUIRE(map.bucket_size(bucket1) == 1);
        REQUIRE(map.contains(0) == false);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(2) == false);

        map.insert({2, "hello1"});
        map.emplace(piecewise_construct_t{},
            make_tuple(3),
            make_tuple("hello2"));

        REQUIRE(map.contains(2));
        REQUIRE(map.contains(3));

        REQUIRE(map.count(0) == 0);
        REQUIRE(map.count(1) == 1);
        REQUIRE(map.count(2) == 1);
        REQUIRE(map.count(3) == 1);

        // Duplicates not permitted on this flavor of emplace
        REQUIRE(map.emplace(2, "hello1.1").second == false);
        REQUIRE(map.emplace(4, "hello3").second);

        // Overriding and permitting duplicate for this guy
        // NOTE: Would fail if bucket_depth wasn't > 1, since 1 and 3 buckets are adjacent
        REQUIRE(map.insert({2, "hello1.1"}, true).second);

        REQUIRE(map.count(2) == 2);
        REQUIRE(map[2] == "hello1");
        iter it_bucket_2 = map.begin(map.bucket(2));
        REQUIRE(it_bucket_2->second == "hello1");
        REQUIRE((it_bucket_2 + 1)->second == "hello1.1");
        map.erase(it_bucket_2);
        REQUIRE(map.count(2) == 1);
        REQUIRE(map[2] == "hello1.1");
        map.erase(it_bucket_2);
        REQUIRE(map.count(2) == 0);
    }
}
