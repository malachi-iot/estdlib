#include <estd/internal/buffer.h>

#include "test-data.h"

#include <catch.hpp>

using namespace estd::internal;

TEST_CASE("buffers")
{
    SECTION("layer2")
    {
        SECTION("ptr")
        {
            uint8_t* ptr = estd::test::octet_data;

            layer2::mutable_buffer<10> b(ptr);

            REQUIRE(sizeof(b) == sizeof(uint8_t*));
        }
        SECTION("array")
        {
            layer2::mutable_buffer<10> b(estd::test::octet_data);

            REQUIRE(sizeof(b) == sizeof(uint8_t*));
        }
    }
    SECTION("A")
    {
        layer3::mutable_buffer b(estd::test::octet_data);

        REQUIRE(sizeof(b) == sizeof(uint8_t*) + sizeof(size_t));
    }
    SECTION("mutable_buffer")
    {
        uint8_t buf[128];
        layer3::mutable_buffer mb(buf);

        REQUIRE(mb.size() == 128);
    }
}
