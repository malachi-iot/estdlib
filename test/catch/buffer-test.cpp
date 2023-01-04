#include <estd/span.h>
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
    SECTION("empty/default constructed")
    {
        estd::span<char> s;

        REQUIRE(s.data() == NULLPTR);
    }
    SECTION("A")
    {
        estd::span<uint8_t> b(estd::test::octet_data);

        REQUIRE(sizeof(b) == sizeof(uint8_t*) + sizeof(size_t));
    }
    SECTION("mutable_buffer")
    {
        uint8_t buf[128];
        estd::span<uint8_t> mb(buf);

        REQUIRE(mb.size() == 128);
    }
    SECTION("span")
    {
        uint8_t buf[128];

        SECTION("dynamic (default)")
        {
            estd::span<uint8_t> s(buf);
            int sz = sizeof(s);

            REQUIRE(sz == sizeof(void*) + sizeof(size_t));
            REQUIRE(s.size() == sizeof(buf));
            REQUIRE(s.data() == &buf[0]);
        }
        SECTION("constexpr-sized")
        {
            estd::span<uint8_t, 128> s(buf);
            int sz = sizeof(s);

            REQUIRE(sz == sizeof(void*));
            REQUIRE(s.size() == sizeof(buf));
            REQUIRE(s.data() == &buf[0]);
        }
        SECTION("dynamic with explicit size")
        {
            estd::span<uint8_t> s(buf, sizeof(buf));
            int sz = sizeof(s);

            REQUIRE(sz == sizeof(void*) + sizeof(size_t));
            REQUIRE(s.size() == sizeof(buf));
            REQUIRE(s.data() == &buf[0]);
        }
        SECTION("subspan")
        {
            //constexpr uint8_t buf2[] = "hi2u";

            estd::span<uint8_t, 128> s(buf);

            s[0] = 77;
            s[1] = 78;
            s[2] = 79;
            s[3] = 80;

            auto s2 = s.subspan(3, 10);

            REQUIRE(s2[0] == 80);
        }
        SECTION("as bytes")
        {
            constexpr int sz = 32;
            int buf[sz];

            buf[0] = 0;
            buf[1] = -1;

            estd::span<int, sz> s(buf);

            auto s2 = estd::as_bytes(s);

            REQUIRE(s2.size() == sizeof(int) * sz);

            estd::byte ch = s2[4];

            REQUIRE(estd::to_integer<int>(ch) == 0xFF);

            SECTION("interact with our special const_buffer")
            {
                //estd::const_buffer b(s2);
            }
        }
        SECTION("as bytes (dynamic)")
        {
            constexpr int sz = 32;
            int buf[sz];

            estd::span<int> s(buf);

            auto s2 = estd::as_bytes(s);

            REQUIRE(s2.size() == sizeof(int) * sz);
        }
    }
}
