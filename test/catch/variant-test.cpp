#include <catch.hpp>

#include <estd/internal/variant.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

struct test_init_functor
{
    template <class T, class TVariant>
    constexpr bool operator()(in_place_type_t<T>, TVariant) const { return false; }

    template <class TVariant>
    bool operator()(in_place_type_t<int>, TVariant& v)
    {
        get<int>(v) = 10;
        return true;
    }
};

struct test_functor
{
    template <unsigned I, class T>
    bool operator()(internal::visitor_index<I, T>, int&& param)
    {
        return param == I;
    }
};

struct tuple_getter_functor
{
    template <unsigned I, class T, class ...TArgs>
    T& operator()(internal::visitor_index<I, T>, tuple<TArgs...>& t)
    {
        return get<I>(t);
    }
};

struct variant_storage_getter_functor
{
    template <unsigned I, class T, class ...TArgs>
    T& operator()(internal::visitor_index<I, T>, internal::variant_storage<TArgs...>& t)
    {
        return get<I>(t);
    }
};


struct test2_functor
{
    template <unsigned I>
    constexpr bool operator()(in_place_index_t<I>) const { return false; }

    template <unsigned I>
    bool operator()(internal::visitor_instance<I, const char*> v)
    {
        return true;
    }
};

TEST_CASE("variant")
{
    SECTION("main")
    {
        typedef internal::variant<int, test::NonTrivial, const char*> variant1_type;

        SECTION("default ctor")
        {
            variant1_type v;
            // Properly doesn't compile, since default value init goes for
            // first item
            //internal::variant<test::NonTrivial, int> v2;

            int& vref = get<int>(v);

            REQUIRE(v.index() == 0);
            REQUIRE(vref == 0);
            REQUIRE(internal::holds_alternative<int>(v));
        }
        SECTION("emplace")
        {
            {
                variant1_type v;

                v.emplace<test::NonTrivial>(7);

                REQUIRE(v.index() == 1);
                REQUIRE(get<test::NonTrivial>(v).code_ == 7);

                REQUIRE(internal::get_if<0>(v) == nullptr);
                REQUIRE(internal::get_if<1>(v) != nullptr);
                REQUIRE(internal::get_if<1>(v)->code_ == 7);

                REQUIRE(internal::get_if<int>(v) == nullptr);
                REQUIRE(internal::get_if<test::NonTrivial>(v) != nullptr);
            }

            // DEBT: Was already 1 from expected-test, now 2 - cross testing globals like this
            // a potential headache
            REQUIRE(test::NonTrivial::dtor_counter == test::dtor_count_2());

            {
                variant1_type v(estd::in_place_type_t<test::NonTrivial>{}, 7);

                v.emplace<int>(10);
                v.emplace<test::NonTrivial>(9);
            }

            REQUIRE(test::NonTrivial::dtor_counter == test::dtor_count_2() + 2);
        }
        SECTION("assign")
        {
            variant1_type v;

            v = 8;

            REQUIRE(get<int>(v) == 8);
        }
        SECTION("copy")
        {
            variant1_type v;

            v = 8;

            variant1_type v2(v);

            REQUIRE(v2.index() == v.index());
            REQUIRE(get<int>(v2) == 8);
        }
        SECTION("move")
        {
            variant1_type v;

            v = 8;

            variant1_type v2(std::move(v));

            REQUIRE(test::NonTrivial::dtor_counter == test::dtor_count_2() + 2);
        }
#if __cpp_concepts
        SECTION("converting constructor")
        {
            variant1_type v("hello");

            REQUIRE(v.index() == 2);
        }
#endif
    }
    SECTION("storage")
    {
        typedef estd::internal::variant_storage<estd::monostate, int> vs_type;

        SECTION("ensure")
        {
            vs_type vs;

            vs_type::ensure_type_t<int> v1;
            vs.get<monostate>();

            // As expected, does not and should not compile
            //vs_type::ensure_type_t<float> v2;
            //vs.get<float>();
        }
        SECTION("monostate, int")
        {
            estd::internal::variant_storage<estd::monostate, int> vs;

            REQUIRE(vs.is_trivial);
        }
        SECTION("int, int")
        {
            estd::internal::variant_storage<int, int> vs;

            REQUIRE(vs.is_trivial);

            *vs.get<0>() = 7;

            auto& v = estd::internal::get<0>(vs);

            REQUIRE(v == 7);
        }
        SECTION("NonTrivial, int")
        {
            internal::variant_storage<estd::test::NonTrivial, int> vs;

            REQUIRE(!vs.is_trivial);
        }
        SECTION("variant_size")
        {
            constexpr unsigned size = internal::variant_size<vs_type>::value;

            REQUIRE(size == 2);
        }
        SECTION("variant_alternative")
        {
            typedef internal::variant_alternative_t<0, vs_type> type0;
            typedef internal::variant_alternative_t<1, vs_type> type1;

            internal::ensure_type_t<monostate, type0> dummy;
            internal::ensure_type_t<int, type1> dummy2 = 0;

            REQUIRE(is_same<monostate, type0>::value);
            REQUIRE(is_same<int, type1>::value);
        }
        SECTION("visitor constructor")
        {
            std::size_t index = -2;
            vs_type v(internal::in_place_visit_t{}, test_init_functor{}, &index);

            REQUIRE(index == 1);
            REQUIRE(get<1>(v) == 10);
        }
    }
    SECTION("misc")
    {
        SECTION("index_of_type")
        {
            SECTION("basic")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, int, float> iot;
                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 1);
                REQUIRE(multiple == false);
            }
            SECTION("not found")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, float> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == -1);
                REQUIRE(multiple == false);
            }
            SECTION("multiple")
            {
                typedef estd::internal::index_of_type<int, int, int> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 0);
                REQUIRE(multiple == true);
            }
        }
        SECTION("visitor")
        {
            SECTION("static")
            {
                typedef internal::variadic_visitor_helper2<monostate, int, float, const char*> vh_type;

                int result = vh_type::visit(test_functor{}, 1);

                REQUIRE(result == 1);
            }
            SECTION("tuple instance")
            {
                typedef internal::variadic_visitor_helper2<float, const char*, int> vh_type;

                tuple<float, const char*, int> t(1.2, "hello", 7);

                int result = vh_type::visit_instance(test2_functor{}, tuple_getter_functor{}, t);

                REQUIRE(result == 1);
            }
            SECTION("variant instance")
            {
                typedef internal::variadic_visitor_helper2<monostate, int, float, const char*> vh_type;

                internal::variant<monostate, int, float, const char*> v;

                int result = vh_type::visit_instance(test2_functor{}, variant_storage_getter_functor{}, v);

                REQUIRE(result == 3);
            }
        }
    }
}

#include "macro/pop.h"
