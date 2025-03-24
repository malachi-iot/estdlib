#include <catch2/catch_all.hpp>

#include <estd/internal/macros.h>


struct NoDiscarder
{
    int v_;

    ESTD_CPP_ATTR_NODISCARD int get() const { return v_; }
};


TEST_CASE("macros")
{
    SECTION("fallthrough")
    {
        int v = 5;

        switch(v)
        {
            case 5:
                ESTD_CPP_ATTR_FALLTHROUGH;

            default:
                ++v;
                break;
        }

        REQUIRE(v == 6);
    }
    SECTION("nodiscard")
    {
        NoDiscarder v{5};

        REQUIRE(v.get() == 5);
    }
}
