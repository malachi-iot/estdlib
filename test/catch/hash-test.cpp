#include <catch2/catch_all.hpp>

#include <estd/internal/functional/hash.h>
#include <estd/string.h>

#include "test-data.h"

using namespace estd;

TEST_CASE("hash")
{
    SECTION("fowler-noll-vo")
    {
        using namespace estd::internal;

        // Verified with https://fnvhash.github.io/fnv-calculator-online/

        SECTION("32-bit")
        {
            uint32_t v = fnv_hash<uint32_t, FNV_1>::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0xa444d06);

            v = fnv_hash<uint32_t, FNV_1A>::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0x82eef4cc);
        }
        SECTION("64-bit")
        {
            auto v = fnv_hash<uint64_t, FNV_1>::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0xb11d013568a3b7c6);

            v = fnv_hash<uint64_t, FNV_1A>::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0xd824008dad4b6b6c);
        }
    }
    SECTION("string")
    {
        // Verified with https://fnvhash.github.io/fnv-calculator-online/

        constexpr estd::layer2::const_string s = "hi2u";

        unsigned hashed = estd::hash<estd::layer2::const_string>{}(s);

        REQUIRE(hashed == 0x4c0a9277);
    }
}