#include <catch2/catch_all.hpp>

// Non-kitchen-sink approach.  Heavily versioning rudimentary memory pools
// so we can progress forward without breaking API

#include <estd/internal/memory/pool.h>

#include "test-data.h"

TEST_CASE("memory pool")
{
    using namespace estd::internal;
    using namespace estd::test;

    int counter = 0;

    SECTION("untyped")
    {
        memory::v1::pool<64, 4> pool;

        SECTION("permutations")
        {
            void* v1 = pool.alloc();
            void* v2 = pool.alloc();
            void* v3 = pool.alloc();

            pool.free(v1);
            pool.free(v2);

            v1 = pool.alloc();

            pool.free(v3);
            pool.free(v1);

            REQUIRE(pool.available_blocks() == pool.max_blocks());

            int8_t h2 = pool.to_handle(v2);
            REQUIRE(pool.from_handle(h2) == v2);
            int8_t h3 = pool.to_handle(v3);
            REQUIRE(pool.from_handle(h3) == v3);

            REQUIRE(unsigned(h2) < pool.max_blocks());
            REQUIRE(unsigned(h3) < pool.max_blocks());
        }
        SECTION("limit")
        {
            pool.alloc();
            pool.alloc();
            pool.alloc();
            pool.alloc();

            REQUIRE(pool.full());
        }
    }
    SECTION("typed")
    {
        memory::v1::typed_pool<Dummy, 4> pool;

        Dummy* dummy1 = pool.emplace();

        REQUIRE(dummy1 != nullptr);
        REQUIRE(pool.available_blocks() == pool.max_blocks() - 1);

        REQUIRE(dummy1->copied_ == false);

        Dummy* dummy2 = pool.emplace(123, "456", &counter);

        REQUIRE(dummy2 != nullptr);

        pool.erase(dummy1);

        REQUIRE(counter == 0);

        REQUIRE(pool.available_blocks() == pool.max_blocks() - 1);

        pool.erase(dummy2);

        REQUIRE(counter == 1);

        REQUIRE(pool.available_blocks() == pool.max_blocks());
    }
}

