#include <catch2/catch_all.hpp>

#include <estd/exp/memory/memory_pool2.h>
#include "../test-data.h"

TEST_CASE("experimental: memory pool v2 tests")
{
    using namespace estd;
    using namespace estd::experimental;

    SECTION("pool")
    {
        typedef array<byte, 32> value_type;

        memory::v2::memory_pool<value_type, 10, memory::v2::pool_types::linked_list> pool;

        REQUIRE(pool.count_free() == 10);
        value_type* v = pool.allocate();
        REQUIRE(pool.count_free() == 9);
        pool.deallocate(v);
        REQUIRE(pool.count_free() == 10);
    }
    SECTION("virtual allocator")
    {
        constexpr int pool_size = 1024;
        memory::v2::virtual_memory<pool_size, 20> v;
        unsigned node_counter;

        SECTION("allocate 1 item")
        {
            int index1 = v.allocate(10);

            v.free(index1);

            REQUIRE(v.total_free(&node_counter) == pool_size);
            REQUIRE(node_counter == 2);
        }
        SECTION("allocate 2 items")
        {
            int index1 = v.allocate(10);
            int index2 = v.allocate(20);

            v.free(index1);
            v.free(index2);

            REQUIRE(v.total_free(&node_counter) == pool_size);
            REQUIRE(node_counter == 3);
        }
        SECTION("zero maint")
        {
            v.maint();

            REQUIRE(v.total_free(&node_counter) == pool_size);
            REQUIRE(node_counter == 1);
        }
        SECTION("1 item maint")
        {
            int index1 = v.allocate(10);

            v.free(index1);

            v.maint();

            REQUIRE(v.total_free(&node_counter) == pool_size);
            REQUIRE(node_counter == 1);
        }
        SECTION("2 item maint")
        {
            int index1 = v.allocate(10);
            int index2 = v.allocate(20);

            v.free(index1);
            v.free(index2);

            // FIX: problematic
            do
            {
                v.maint();
            }
            while(v.state() != memory::v2::maintenance_mode::uninitialized);

            REQUIRE(v.total_free(&node_counter) == pool_size);
            //REQUIRE(node_counter == 1);
        }
    }
}
