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
struct access_experiment;

template <int I, class ...Types>
internal::type_at_index<I, Types...>& get_exp(access_experiment<Types...>& v)
{
    return * v.template get<I>();
}


template <class ...Types>
struct access_experiment : protected internal::variant_storage<Types...>
{
    using this_type = access_experiment;
    typedef internal::variant_storage<Types...> base_type;

    ESTD_CPP_FORWARDING_CTOR(access_experiment);

    // Not good enough, compiler doesn't resolve this friend
    //template <int I>
    //friend internal::type_at_index<I, Types...>& ::get_exp(access_experiment<Types...>& v);

    template <int I, class ...Types2>
    friend internal::type_at_index<I, Types2...>& ::get_exp(access_experiment<Types2...>& v);

    // No dice, because implicit cast down to variant_storage happens "just before" call
    template <int I, class ...Types2>
    friend internal::type_at_index<I, Types2...>& estd::internal::get(internal::variant_storage<Types2...>& vs);
};


TEST_CASE("variant")
{
    SECTION("main")
    {
        typedef variant<int, test::NonTrivial, const char*> variant1_type;

        SECTION("default ctor")
        {
            variant1_type v;
            // Properly doesn't compile, since default value init goes for
            // first item
            //internal::variant<test::NonTrivial, int> v2;

            int& vref = get<int>(v);

            REQUIRE(v.index() == 0);
            REQUIRE(vref == 0);
            REQUIRE(holds_alternative<int>(v));
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

                REQUIRE(get_if<0>(&v) == nullptr);
                REQUIRE(get_if<1>(&v) != nullptr);
                REQUIRE(get_if<1>(&v)->code_ == 7);

                REQUIRE(get_if<int>(&v) == nullptr);
                REQUIRE(get_if<test::NonTrivial>(&v) != nullptr);
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
            REQUIRE(counter == 0);

            {
                variant1_type v2(estd::in_place_type_t<test::NonTrivial>{}, 7, dtor_fn);

                // sneakily NonTrivial has no assignment operator, so we need a direct initialize
                // However, destruct is basically a noop since active variant is the integer
                v = v2;

                REQUIRE(counter == 0);

                {
                    // Makes a copy, so expect another dtor
                    auto _v = get<test::NonTrivial>(v);

                    REQUIRE(_v.code_ == 7);
                }

                REQUIRE(counter == 1);

                {
                    // Brand new NonTrivial, so dtor is gonna bump up counter
                    test::NonTrivial v3(7, dtor_fn);

                    // dtor runs here, because there is no assignment operator
                    // so full destruct/di happens
                    v = v3;
                }

                REQUIRE(counter == 3);

                {
                    variant1_type v3(in_place_index_t<1>{}, 8, dtor_fn);
                    test::NonTrivial& v4 = get<1>(v3);

                    REQUIRE(get<1>(v3).initialized_ == true);
                    REQUIRE(get<1>(v3).moved_ == false);
                    REQUIRE(get<1>(v3).moved_from_ == false);

                    // since no assignment operator, existing 'v' dtor runs
                    // v3 dtor also runs, but dtor_fn does NOT, because move on a std::function
                    // clears out the target
                    v = std::move(v3);

                    REQUIRE(v3.valueless_by_exception());

                    // NOTE: In this narrow case, looking at destructed data
                    // is still OK (not undefined)
                    REQUIRE(v4.initialized_ == true);
                    REQUIRE(v4.moved_ == false);
                    REQUIRE(v4.moved_from_);
                    REQUIRE(v4.destroyed_);
                    REQUIRE(v4.on_dtor == nullptr);

                    REQUIRE(counter == 4);
                    REQUIRE(get<1>(v).code_ == 8);
                    REQUIRE(get<1>(v).moved_);
                    REQUIRE(get<1>(v).copied_ == false);
                    REQUIRE(get<1>(v).on_dtor != nullptr);
                }

                REQUIRE(counter == 4);

                {
                    variant1_type v3(5);

                    // wipes out NonTrivial v alternative, resulting in yet another
                    // dtor_fn call
                    v = v3;

                    REQUIRE(counter == 5);
                }

                // v2 dtor runs here, bumping us up to 6
            }

            REQUIRE(counter == 6);
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

                REQUIRE(get<test::NonTrivial>(v3).code_ == 1);
                REQUIRE(get<test::NonTrivial>(v3).copied_);
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
        SECTION("converting constructor")
        {
            SECTION("integer")
            {
                variant1_type v{1};

                REQUIRE(v.index() == 0);
            }
            SECTION("string")
            {
                variant1_type v{"hello"};

                REQUIRE(v.index() == 2);
            }
        }
        SECTION("duplicate type")
        {
            variant<int, int, const int> v;

            REQUIRE(get<0>(v) == 0);

            get<int>(v) = 1;

            int index = decltype(v)::select_type<const int>::index;

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

            REQUIRE(get_if<int>(v) == nullptr);
            REQUIRE(get_if<0>(v) == nullptr);

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

            unsigned sz = variant_size<decltype(vs)>::value;

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
            constexpr unsigned size = variant_size<vs_type>::value;

            REQUIRE(size == 2);
        }
        SECTION("variant_alternative")
        {
            typedef variant_alternative_t<0, vs_type> type0;
            typedef variant_alternative_t<1, vs_type> type1;

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
        SECTION("single instance")
        {
            internal::instance_storage<test::NonTrivial> storage;

            storage.emplace(0);

            REQUIRE(storage.get()->initialized_ == true);
            REQUIRE(storage.get()->destroyed_ == false);

            storage.destroy();

            REQUIRE(storage.get()->destroyed_);
        }
    }
    SECTION("experimental")
    {
        access_experiment<int, monostate> v(in_place_index_t<0>{}, 0);

        //int value = internal::get<0>(v);
        int value = get_exp<0>(v);

        //value = internal::get<0>(v);
    }
}

#include "macro/pop.h"
