#include <catch2/catch_all.hpp>

#include <estd/internal/rtto.h>
#include <estd/span.h>
#include <estd/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

// Not really useful in the real world, since T is more direct than CREATE.  But useful here for RTTO testing.
template <class T>
T* default_create(internal::instance_storage<T>& storage, int* ret)
{
    using rtto = internal::rtto<T, internal::rtto_traits<T>>;
    *ret = rtto::utility(internal::rtto_modes::CREATE, storage.get(), sizeof(T), nullptr);
    return storage.get();
}


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
        REQUIRE(d2.inc_on_destruct == &destruct_counter);
        REQUIRE(destruct_counter == 0);

        int rc = move_to_and_destroy(u, &d2, &d3);

        // NOTE: d2 is destroyed here, which gives us partial UB since d2 auto destructs
        // and end of scope too.  test::Dummy is simplistic so this causes us no issues
        REQUIRE(rc == 0);

        // Still 0 because move nulls out inc_on_destruct pointer
        REQUIRE(destruct_counter == 0);

        // DEBT: Not working right because destruct_counter is not intuitive with dest::Dummy.
        // Needs some love
        //REQUIRE(destruct_counter == 1);
        REQUIRE(d1.destroyed_ == false);    // Moved but not destroyed yet

        // Inspecting values after destruction is UB.  It works OK in Debug mode.
#if !NDEBUG
        REQUIRE(d2.destroyed_);
#endif
        REQUIRE(d3.destroyed_ == false);
    }
    SECTION("test::NonCopyable")
    {
        using type = test::NonCopyable;
        using storage = internal::instance_storage<type>;
        using rtto = internal::rtto<type>;

        storage s;

        REQUIRE(size(rtto::utility) == sizeof(type));
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

        // Inspecting values after destruction is UB.  It works OK in Debug mode.
#if !NDEBUG
        REQUIRE(s1->destroyed_);
#endif
    }
    SECTION("No Defaut Constructor")
    {
        SECTION("basic case")
        {
            using type = test::NoDefaultConstructor;
            using storage = internal::instance_storage<type>;

            storage s1;

            int ret{};

            default_create(s1, &ret);

            REQUIRE(ret == EINVAL);
        }
        SECTION("forwarded")
        {
            using type = test::Forwarder<test::NoDefaultConstructor>;
            using storage = internal::instance_storage<type>;

            storage s1;

            int ret{};

            default_create(s1, &ret);

            REQUIRE(ret == EINVAL);

            // Noting peculiarity where overload resolution says this is default constructible, even though it isn't
            static_assert(std::is_default_constructible<type>{}.value, "");
            static_assert(std::is_trivially_default_constructible<type>{}.value == false, "");
        }
    }
    SECTION("Proxy")
    {
        using type = test::Dummy;
        //using storage = internal::instance_storage<type>;

        // Brute forcing to activate experimental CREATE mode
        // FIX: If using estd::bool_constant, this dies, presumably because we don't alias it out
        // See https://github.com/malachi-iot/estdlib/issues/164 - matters because not everyone
        // is on c++17
        using rtto = internal::rtto<type, internal::rtto_traits<type, bool_constant<true>>>;
        alignas(void*) char storage[64] {};
        alignas(void*) char storage2[64] {};

        SECTION("inline (layer1 style)")
        {
            using proxy = internal::rtto_base::proxy<char[]>;

            auto p = (proxy*) storage;  // NOLINT
            new (storage) proxy(rtto::utility);
            int r = p->create();        // EXPERIMENTAL, but great for testing
            auto v = p->storage<type*>();

            REQUIRE(r == 0);
            REQUIRE(v->initialized_);
        }
        SECTION("move constructor")
        {
            int counter = 0;
            int rc = -1;

            using proxy = internal::rtto_base::proxy<>;

            auto p1 = (proxy*) storage;  // NOLINT
            auto p2 = (proxy*) storage2;  // NOLINT

            //new (p1) proxy(rtto::utility);
            //auto dummy1 = new (p1->storage()) type(7, "hello", &counter);
            new (p1) proxy(estd::in_place_type_t<type>{}, 7, "Hello", &counter);
            auto dummy1 = (type*) p1->storage();

            REQUIRE(dummy1->moved_ == false);
            REQUIRE(dummy1->moved_from_ == false);

            new (p2) proxy(std::move(*p1), &rc);

            REQUIRE(rc == 0);

            auto dummy2 = (type*) p2->storage();

            REQUIRE(dummy1->moved_from_ == true);
            REQUIRE(dummy2->moved_);

            REQUIRE(counter == 0);

            p1->destroy();
            p2->destroy();

            REQUIRE(counter == 1);
        }
        SECTION("copy constructor")
        {
            int rc = -1;
            int counter = 0;
            using proxy = internal::rtto_base::proxy<>;

            auto p1 = (proxy*) storage;  // NOLINT
            auto p2 = (proxy*) storage2;  // NOLINT

            //new (p1) proxy(rtto::utility);
            //auto dummy1 = new (p1->storage()) type(7, "hello", &counter);
            new (p1) proxy(estd::in_place_type_t<type>{}, 7, "Hello", &counter);

            new (p2) proxy(*p1, &rc);

            REQUIRE(rc == 0);

            auto dummy1 = static_cast<type*>(p1->storage());
            auto dummy2 = static_cast<type*>(p2->storage());

            REQUIRE(dummy1->copied_ == false);
            REQUIRE(dummy2->copied_);

            p1->destroy();
            p2->destroy();

            REQUIRE(counter == 2);
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
    SECTION("virtual")
    {
        int counter = 0;

        test::VirtualRttoDummy vrd1(7, "vrd1", &counter);
        test::VirtualRttoDummy vrd2;

        REQUIRE(vrd1.initialized_);

        vrd1.move_to(&vrd2, 0);

        REQUIRE(vrd1.moved_from_);
        REQUIRE(vrd2.moved_);
    }
}

#include "macro/pop.h"
