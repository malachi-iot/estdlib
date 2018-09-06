#include <estd/span.h>

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
        estd::mutable_buffer b(estd::test::octet_data);

        REQUIRE(sizeof(b) == sizeof(uint8_t*) + sizeof(size_t));
    }
    SECTION("mutable_buffer")
    {
        uint8_t buf[128];
        estd::mutable_buffer mb(buf);

        REQUIRE(mb.size() == 128);
    }
    SECTION("span")
    {
        SECTION("dynamic (default)")
        {
            uint8_t buf[128];
            estd::span<uint8_t> s(buf);
            int sz = sizeof(s);

            REQUIRE(sz == sizeof(void*) + sizeof(size_t));
            REQUIRE(s.size() == sizeof(buf));
            REQUIRE(s.data() == &buf[0]);
        }
        SECTION("constexpr-sized")
        {
            uint8_t buf[128];
            estd::span<uint8_t, 128> s(buf);
            int sz = sizeof(s);

            REQUIRE(sz == sizeof(void*));
            REQUIRE(s.size() == sizeof(buf));
            REQUIRE(s.data() == &buf[0]);
        }
    }
}
