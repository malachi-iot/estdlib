#include <catch2/catch_all.hpp>

#include <estd/functional.h>

#include "../test-data.h"

#include "../macro/push.h"

using namespace estd;

template <class T>
T passthru(T v) { return v; }

template <typename T>
using function = detail::v2::function<T, detail::impl::function_virtual>;


TEST_CASE("functional: virtual", "[functional]")
{
    using fn_int = function<int(int)>;

    SECTION("functor_provider") // DEBT: This test belongs elsewhere
    {
        int counter = 1;

        auto f_cap = [&](int v) { return counter + v; };    // NOLINT
        auto f_nocap = [](int v) { return v; };

        detail::impl::functor_provider<decltype(f_cap)> fp_cap(std::move(f_cap));
        detail::impl::functor_provider<decltype(f_nocap)> fp_nocap(std::move(f_nocap));

        REQUIRE(fp_cap.functor()(5) == 6);
        REQUIRE(fp_nocap.functor()(5) == 5);

        static_assert(sizeof(fp_cap) == sizeof(void*), "function_provider should be size of a reference (pointer)");
        static_assert(sizeof(fp_nocap) < sizeof(void*), "function_provider should be '1' (aka empty)");
    }
    SECTION("model")
    {
        SECTION("non-capturing")
        {
            auto m = fn_int::make_model([](int v) { return v * 2; });

            static_assert(sizeof(m) == sizeof(internal::rtto_base::virtual_base), "Only size of vtable");
        }
        SECTION("direct function address")
        {
            //auto m = fn_int::make_model(passthru);
        }
        SECTION("aliased")
        {
            function<void(int&&)> f;

            struct model : decltype(f)::model_base
            {
                int counter = 0;

                void operator()(int&& v) override   // NOLINT experimental only
                {
                    counter += v;
                }
            };
        }
        SECTION("place_model")
        {
            using fn_type = function<void()>;
            fn_type f;
            char raw[64];
            int counter1 = 0;
            int counter2 = 10;

            SECTION("void*")
            {
                void* p = raw;
                f = fn_type::place_model(p, [&counter1, counter2]
                    { counter1 += counter2; });

                f();

                REQUIRE(counter1 == counter2);
            }
            SECTION("array")
            {
                f = fn_type::place_model(raw, [&counter1, counter2]
                    { counter1 += counter2; });

                f();

                REQUIRE(counter1 == counter2);
            }

            //fn_type::place_model([&]());
        }
    }
    SECTION("high level")
    {

    }
}

#include "../macro/pop.h"
