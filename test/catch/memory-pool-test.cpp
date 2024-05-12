#include <catch.hpp>

// Non-kitchen-sink approach.  Heavily versioning rudimentary memory pools
// so we can progress forward without breaking API

#include <estd/internal/memory/v1/pool.h>

#include "test-data.h"

TEST_CASE("memory pool")
{
    using namespace estd::internal;
    using namespace estd::test;

    int counter = 0;

    SECTION("untyped")
    {
        memory::v1::pool<64, 4> pool;
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

