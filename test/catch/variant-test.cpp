#include <catch.hpp>

#include <estd/variant.h>

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


template <class ...Types>
using variant = internal::variant<Types...>;

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
            int counter = 0;
            auto dtor_fn = [&]{ ++counter; };

            {
                variant1_type v;

                v.emplace<test::NonTrivial>(7, dtor_fn);

                REQUIRE(v.index() == 1);
                REQUIRE(get<test::NonTrivial>(v).code_ == 7);

                REQUIRE(internal::get_if<0>(v) == nullptr);
                REQUIRE(internal::get_if<1>(v) != nullptr);
                REQUIRE(internal::get_if<1>(v)->code_ == 7);

                REQUIRE(internal::get_if<int>(&v) == nullptr);
                REQUIRE(internal::get_if<test::NonTrivial>(&v) != nullptr);
            }

            {
                variant1_type v(estd::in_place_type_t<test::NonTrivial>{}, 7, dtor_fn);

                v.emplace<int>(10);
                v.emplace<test::NonTrivial>(9, dtor_fn);

                v.emplace<2>(test::str_hello);

                REQUIRE(v.index() == 2);
                REQUIRE(get<2>(v) == test::str_hello);
            }

            REQUIRE(counter == 3);
        }
        SECTION("assign")
        {
            int counter = 0;
            auto dtor_fn = [&]{ ++counter; };

            variant1_type v;

            v = 8;

            REQUIRE(get<int>(v) == 8);

            {
                variant1_type v2(estd::in_place_type_t<test::NonTrivial>{}, 7, dtor_fn);

                v = v2;

                auto _v = get<test::NonTrivial>(v);

                REQUIRE(_v.code_ == 7);

                test::NonTrivial v3(7, dtor_fn);

                v = v3;
            }

            REQUIRE(counter == 4);
        }
        SECTION("copy")
        {
            int counter = 0;
            auto dtor_fn = [&]{ ++counter; };

            {
                variant1_type v;

                v = 8;

                variant1_type v2(v);

                REQUIRE(v2.index() == v.index());
                REQUIRE(get<int>(v2) == 8);

                v.emplace<test::NonTrivial>(1, dtor_fn);
                v2.emplace<test::NonTrivial>(2, dtor_fn);
                variant1_type v3(v);

                REQUIRE(v3.get<test::NonTrivial>()->code_ == 1);
                REQUIRE(v3.get<test::NonTrivial>()->copied_);
            }

            REQUIRE(counter == 3);
        }
        SECTION("move")
        {
            int counter = 0;
            auto dtor_fn = [&]{ ++counter; };

            {
                variant1_type v;

                v.emplace<test::NonTrivial>(8, dtor_fn);

                variant1_type v2(std::move(v));
            }

            REQUIRE(counter == 1);
        }
#if __cpp_concepts
        SECTION("converting constructor")
        {
            variant1_type v("hello");

            REQUIRE(v.index() == 2);
        }
#endif
        SECTION("duplicate type")
        {
            variant<int, int, const int> v;

            REQUIRE(get<0>(v) == 0);

            get<int>(v) = 1;

            int index = decltype(v)::index_of_type<const int>::index;

            REQUIRE(index == 2);

            // the getter doesn't seem to have any way to "cheat" and get at const int
            // without throwing an exception.  Therefore, I'm not sure why variants can
            // even support duplicates - unless it's to accomodate scnearios where Types...
            // somehow wasn't that controlled in the first place (auto generate for example)
            //REQUIRE(get<const int>(v) == 1);
        }
        SECTION("get_if")
        {
            variant1_type* v = nullptr;

            REQUIRE(internal::get_if<int>(v) == nullptr);

            // "ill-formed"
            //REQUIRE(internal::get_if<float>(v) == nullptr);
        }
    }
    SECTION("storage")
    {
        typedef estd::internal::variant_storage<estd::monostate, int> vs_type;

        SECTION("empty")
        {
            internal::variant_storage<> vs;

            unsigned sz = internal::variant_size<decltype(vs)>::value;

            REQUIRE(sz == 0);
        }
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
            vs_type vs;

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
}

#include "macro/pop.h"
