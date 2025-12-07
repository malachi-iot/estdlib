#include <catch2/catch_all.hpp>

#include <estd/internal/rtto.h>
#include <estd/span.h>
#include <estd/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

TEST_CASE("rtto", "Runtime Type Operations")
{
    using util = internal::rtto_base::utility_type;

    SECTION("test::Dummy")
    {
        using rtto = internal::rtto<test::Dummy>;
        const util u = rtto::utility;
        int destruct_counter = 0;

        test::Dummy d1(1, "Dummy1", &destruct_counter);
        test::Dummy d2, d3;

        move_to(u, &d1, &d2);

        REQUIRE(d1.moved_from_);
        REQUIRE(d2.moved_);

        REQUIRE(d2.inc_on_destruct);
        REQUIRE(destruct_counter == 0);

        // NOTE: d2 is destroyed here, which gives us partial UB since d2 auto destructs
        // and end of scope too.  test::Dummy is simplistic so this causes us no issues
        REQUIRE(move_to_and_destroy(u, &d2, &d3) == 0);

        // DEBT: Not working right because destruct_counter is not intuitive with dest::Dummy.
        // Needs some love
        //REQUIRE(destruct_counter == 1);
        REQUIRE(d1.destroyed_ == false);    // Moved but not destroyed yet
        REQUIRE(d2.destroyed_);
        REQUIRE(d3.destroyed_ == false);
    }
    SECTION("test::NonCopyable")
    {
        using type = test::NonCopyable;
        using storage = internal::instance_storage<type>;
        using rtto = internal::rtto<type>;
        const util u = rtto::utility;

        storage s;
    }
    SECTION("test::NonTrivial")
    {
        using type = test::NonTrivial;
        using storage = internal::instance_storage<type>;

        using rtto = internal::rtto<type>;
        const util u = rtto::utility;

        storage s1, s2;

        s1.emplace(5);

        REQUIRE(s1->destroyed_ == false);

        internal::move_to_and_destroy(u, s1.get(), s2.get());

        REQUIRE(s1->destroyed_);
    }
    SECTION("Proxy")
    {
        using type = test::Dummy;
        //using storage = internal::instance_storage<type>;

        using rtto = internal::rtto<type>;
        alignas(void*) char storage[128] {};

        SECTION("inline (layer1 style)")
        {
            using proxy = internal::rtto_base::proxy<char[]>;

            auto p = (proxy*) storage;  // NOLINT
            new (storage) proxy(rtto::utility);
            int r = p->create();
            auto v = p->storage<type*>();

            REQUIRE(r == 0);
            REQUIRE(v->initialized_);
        }
        SECTION("pointer (layer2 style)")
        {
            using proxy = internal::rtto_base::proxy<char*>;

            proxy p(rtto::utility, storage);

            auto v = p.storage<type*>();

            REQUIRE(v->initialized_ == false);

            int r = p.create();

            REQUIRE(r == 0);
            REQUIRE(v->initialized_);
        }
        SECTION("span (layer3 style)")
        {
            using proxy = internal::rtto_base::proxy<estd::span<char>>;

            proxy p(rtto::utility, storage, sizeof(storage));

            // Almost works.  Would need some fancy specialization.  Can't think of a direct need for layer3
            // style, so leaving it
            //int r = p.create();
        }
    }
}

#include "macro/pop.h"
