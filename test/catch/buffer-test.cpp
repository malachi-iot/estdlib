#include <estd/span.h>
#include <estd/internal/buffer.h>
#include <estd/internal/bip/buffer.h>
#include <estd/internal/iterator/size.h>
#include <estd/expected.h>

#include "test-data.h"

#include <catch2/catch_all.hpp>

template <ESTD_CPP_CONCEPT(estd::concepts::v1::Bipbuf) Bipbuf>
void test_bipbuf(Bipbuf& bip)
{
    using namespace estd::test;
    constexpr unsigned sz = sizeof(octet_data);

    bip.offer(octet_data, sz);
    bip.offer(octet_data, sz);

    REQUIRE(bip.unused() >= sz);

    memcpy(bip.offer_begin(), octet_data, sz);
    bip.offer_end(sz);

    REQUIRE(bip.offer(octet_data, sz) == 0);

    REQUIRE(bip.used() == sz * 3);
    REQUIRE(bip.peek(sz * 3) != nullptr);
    REQUIRE(bip.peek(sz * 4) == nullptr);

    const uint8_t* v = bip.poll(sz * 3);

    REQUIRE(memcmp(octet_data, v, sz) == 0);
    REQUIRE(memcmp(octet_data, v + sz, sz) == 0);
    REQUIRE(memcmp(octet_data, v + sz * 2, sz) == 0);
}

TEST_CASE("buffers")
{
    SECTION("Catch compatibility")
    {
        estd::expected<void, estd::errc> exp;

        // Catch2 is sensitive to begin() treatment.  If this compiles, we're good.
        // See estd::internal::is_container
        CAPTURE(exp);
    }
    SECTION("layer1")
    {
        SECTION("bipbuffer")
        {
            estd::layer1::bipbuf<32> bip;
            test_bipbuf(bip);
        }
    }
    SECTION("layer2")
    {
        using namespace estd::internal;

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
    SECTION("layer3")
    {
        SECTION("bipbuffer")
        {
            union
            {
                estd::byte backing[sizeof(bipbuf_t) + 32];
                bipbuf_t underlying;
            };
            estd::layer3::bipbuf bip(&underlying, 32);
            test_bipbuf(bip);
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
    SECTION("Freestanding")
    {
        constexpr int sz = 32;

        union holder
        {
            const char cbuf[sz]{};
            char buf[sz];
        }   holder1;

        char (&buf)[sz] = holder1.buf;

        holder* holder2 = &holder1;

        // container freestanding assist functions
        SECTION("raw array")
        {
            using traits = estd::internal::container_traits<estd::remove_reference_t<decltype(buf)>>;

            REQUIRE(estd::size(buf) == sz);
            REQUIRE(estd::size(holder2->buf) == sz);
            REQUIRE(estd::data(buf) == buf);
            REQUIRE(estd::data(holder1.cbuf) == buf);
            REQUIRE(estd::begin(buf) == buf);
            REQUIRE(estd::begin(holder1.cbuf) == buf);
            REQUIRE(estd::end(buf) == buf + sz);
            REQUIRE(estd::end(holder1.cbuf) == buf + sz);

            REQUIRE(traits::extent != estd::detail::dynamic_extent());
        }
        SECTION("span")
        {
            SECTION("static extent")
            {
                estd::span<char, sz> s(buf);

                using traits = estd::internal::container_traits<decltype(s)>;

                REQUIRE(traits::extent == sz);

                REQUIRE(estd::size(s) == sz);
                REQUIRE(estd::data(s) == buf);
                REQUIRE(estd::begin(s) == buf);
                REQUIRE(estd::end(s) == buf + sz);
            }
            SECTION("dynamic extent")
            {
                estd::span<char> s(buf);

                using traits = estd::internal::container_traits<decltype(s)>;

                REQUIRE(traits::extent == estd::detail::dynamic_extent());

                REQUIRE(estd::size(s) == sz);
                REQUIRE(estd::data(s) == buf);
                REQUIRE(estd::begin(s) == buf);
                REQUIRE(estd::end(s) == buf + sz);
            }
            SECTION("struct + static extent")
            {
                estd::test::Dummy storage[4];
                estd::span<estd::test::Dummy, sz> s(storage);

                using traits = estd::internal::container_traits<decltype(s)>;

                REQUIRE(traits::extent == sz);
            }
            SECTION("struct + dynamic extent")
            {
                estd::test::Dummy storage[4];
                estd::span<estd::test::Dummy> s(storage);

                using traits = estd::internal::container_traits<decltype(s)>;

                REQUIRE(traits::extent == estd::detail::dynamic_extent());
            }
        }
#if FEATURE_STD_SPAN
        SECTION("std::span")
        {
            std::span<char> s(buf);

            using traits = estd::internal::container_traits<decltype(s)>;

            REQUIRE(traits::extent == estd::detail::dynamic_extent());

            REQUIRE(estd::size(s) == 32);
            REQUIRE(estd::data(s) == buf);
        }
#endif
    }
}
