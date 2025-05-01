#include <catch2/catch_all.hpp>

#include <cmrc/cmrc.hpp>

#include <estd/internal/functional/hash.h>
#include <estd/string.h>
#include <estd/string_view.h>

#include "test-data.h"

CMRC_DECLARE(TESTRC);

using namespace estd;

TEST_CASE("hash")
{
    auto fs = cmrc::TESTRC::get_filesystem();

    SECTION("fowler-noll-vo")
    {
        using namespace estd::internal;

        // Verified with https://fnvhash.github.io/fnv-calculator-online/

        SECTION("32-bit")
        {
            uint32_t v = fnv_hash<uint32_t, FNV_1>::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0xa444d06);

            using fnv1a = fnv_hash<uint32_t, FNV_1A>;

            v = fnv1a::hash(test::octet_data, test::octet_data + 9);

            REQUIRE(v == 0x82eef4cc);

            v = fnv1a::hash((const char*)test::octet_data, (const char*)test::octet_data + 9);

            REQUIRE(v == 0x82eef4cc);

            cmrc::file f1 = fs.open("resources/hello.txt");

            // DEBT: string_view could really use the iterator begin/iterator end constructor
            estd::string_view s(f1.begin(), f1.end() - f1.begin());

            REQUIRE(s == "Hi!");

            v = fnv1a::hash(f1.begin(), f1.end());

            REQUIRE(v == 0x325d0001);
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
