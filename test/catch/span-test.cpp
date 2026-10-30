#include <catch2/catch_all.hpp>

#include <estd/span.h>

TEST_CASE("span")
{
    uint8_t buf[128];

    SECTION("mutable_buffer")
    {
        estd::span<uint8_t> mb(buf);

        REQUIRE(mb.size() == 128);
    }
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
    SECTION("converting constructor")
    {
        estd::span<const uint8_t, sizeof(buf)> s1(buf);
        estd::span<const uint8_t> s2(s1);

        REQUIRE(s2.size() == sizeof(buf));
    }
}