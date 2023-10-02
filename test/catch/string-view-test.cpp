#include <estd/string.h>
#include <estd/string_view.h>
#include <estd/vector.h>
#include <estd/charconv.h>
#include <cstdlib>

#include <catch.hpp>

using namespace estd;

#include "macro/push.h"

TEST_CASE("string_view")
{
    SECTION("string_view")
    {
        string_view sv("test", 4);
        string_view sv2 = sv;
        string_view sv3 = "test3";

        int sz = sizeof(sv);

        // policy makes all strings default to size type of uint16_t
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        REQUIRE(sizeof(string_view::size_type) == sizeof(uint16_t));
#endif
        REQUIRE(sz == sizeof(char*) + sizeof(size_t));

        REQUIRE(sv3.starts_with(sv));
        REQUIRE(sv2 == sv);
        REQUIRE(sv2.compare(sv) == 0);
        REQUIRE(sv2 == "test");

        // NOTE: Only works in an experimental capacity right now.  Adjusted
        // layer3::allocator to have experimental setters to adjust its pointer
        // and size
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        sv3.remove_suffix(2);

        REQUIRE(sv3 == "tes");

        sv3.remove_prefix(1);

        REQUIRE(sv3 == "es");
#endif

        // Does not compile, as is correct behavior - string_views are read only except
        // for remove_suffix and remove_prefix
        //sv3 += "T";

        SECTION("from layer1::string")
        {
            layer1::string<32> s = "hi2u";

            estd::string_view v = s;

            REQUIRE(v == "hi2u");
        }
        SECTION("from layer2::string")
        {
            layer2::const_string s = "hi2u";

            estd::string_view v = s;

            REQUIRE(v == "hi2u");
        }
        SECTION("from layer3::string")
        {
            layer3::const_string s = "hi2u";

            estd::string_view v = s;

            REQUIRE(v == "hi2u");
        }

        sv3 = "test3";

        estd::string_view sv4 = sv3.substr(1, 2);

        const char* helper = sv4.lock();

        REQUIRE(sv4 == "es");
    }
    SECTION("find")
    {
        string_view sv("key=value");
        unsigned pos = sv.find('=');

        REQUIRE(pos == 3);

        string_view sv_key = sv.substr(0, pos);
        string_view sv_value = sv.substr(pos + 1);

        REQUIRE(sv_key == "key");
        REQUIRE(sv_value == "value");
    }
    SECTION("constexpr")
    {
        constexpr string_view sv("test", 4);
    }
}


#include "macro/pop.h"
