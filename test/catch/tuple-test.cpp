#include <catch2/catch.hpp>

#include <estd/tuple.h>
#include <estd/internal/variadic.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;
using namespace estd::test;

template <class ...TArgs>
void tester(TArgs...args)
{

}

template <class ...TArgs>
struct tester2
{
    tuple<TArgs...> v;

    explicit tester2(TArgs&&...args) : v(std::forward<TArgs>(args)...)
    {

    }

    float adder() { return 0; }

    template <class ...TArgs2>
    float adder(float v, TArgs2 const&...args)
    {
        return v + adder(args...);
    }

    // DEBT: Would be better if we could return auto here, but c++11 doesn't go for
    // that
    float test()
    {
        return apply([&](TArgs const&... tupleArgs)
        {
            return adder(tupleArgs...);
        }, v);
    }
};


template <class Tuple, size_t... Is>
constexpr auto take_front_impl(const Tuple& t,
    index_sequence<Is...>) ->
decltype (make_tuple(get<Is>(t)...))
{
    return make_tuple(get<Is>(t)...);
}

template <size_t N, class Tuple>
constexpr auto take_front(const Tuple& t) ->
decltype (take_front_impl(t, make_index_sequence<N>{}))
{
    return take_front_impl(t, make_index_sequence<N>{});
}



TEST_CASE("tuple")
{
    constexpr auto z = estd::make_tuple(1, 2);

    SECTION("std lib version")
    {
        std::tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("estd lib version")
    {
        using namespace estd;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("minimal tuple")
    {
        auto tuple = estd::make_tuple(false, true);

        int sz = sizeof(tuple);

        REQUIRE(sz == 2);

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == false);
    }
    SECTION("tuple")
    {
        auto tuple = estd::make_tuple(0, 1.0, 2);

        // Size checks are very platform/toolchain dependent
        // DEBT: Something is off here, Raspbian 32 bit deduces sz
        // to be 24 and right side as 16.  Makes me think make_tuple
        // is using too large of integer values - or packing things funny
#if defined(ESTD_OS_UNIX) && ESTD_ARCH_BITNESS == 64
        int sz = sizeof(tuple);

        REQUIRE(sz == sizeof(long) * 2 + sizeof(double));
#endif

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == 0);

        auto val2 = estd::get<1>(tuple);

        REQUIRE(val2 == 1.0);

        auto val3 = estd::get<2>(tuple);

        REQUIRE(val3 == 2);

        typedef estd::tuple_size<decltype (tuple)> test_type;
        int val = test_type::value;
        REQUIRE(val == 3);
        // Following doesn't work, catch must try to get the address of value
        //REQUIRE(estd::tuple_size<decltype (tuple)>::value == 3);

        SECTION("take front")
        {
            auto tuple2 = take_front<2>(tuple);
            val = estd::tuple_size<decltype (tuple2)>::value;

            REQUIRE(val == 2);

            REQUIRE(get<1>(tuple2) == 1.0);

            // Compile time error, as it should be:
            //estd::get<2>(tuple2);
        }

        typedef estd::tuple_element<0, decltype (tuple)>::type te;

        bool is_same = estd::is_same<te, int>::value;

        REQUIRE(is_same);

        is_same = estd::is_same<estd::tuple_element_t<1, decltype (tuple)>, double>::value;

        REQUIRE(is_same);

        is_same = estd::is_same<estd::tuple_element_t<2, decltype (tuple)>, int>::value;

        REQUIRE(is_same);

        WHEN("reference tests")
        {
            int& v1 = get<0>(tuple);

            REQUIRE(v1 == val1);

            double& v2 = get<1>(tuple);

            REQUIRE(v2 == val2);

            // FIX: Not quite working
            //double& v3 = get<double>(tuple);

            //REQUIRE(v3 == v2);
        }
        WHEN("more reference tests")
        {
            int int_value = 7;
            estd::tuple<int&> tuple3(int_value);

            int& int_ref = get<0>(tuple3);

            REQUIRE(int_ref == 7);

            int_ref = 5;

            REQUIRE(int_value == 5);

            tuple3.visit([](estd::variadic::v2::instance<0, int&> i)
            {
                i.value = 10;
                return false;
            });

            REQUIRE(int_value == 10);
        }
        SECTION("multiple identical")
        {
            estd::tuple<
                Templated<0>,
                Templated<0>,
                Templated<0> > tuple3;

            get<0>(tuple3).val1 = 1;
            get<1>(tuple3).val1 = 2;
            get<2>(tuple3).val1 = 2;

            // properly doesn't compile, as per https://en.cppreference.com/w/cpp/utility/tuple/get
            //get<Templated<0>>(tuple3).val1 = 2;
        }
    }
    SECTION("value initialization")
    {
        using namespace estd;

        // As per https://en.cppreference.com/w/cpp/utility/tuple/tuple, tuples
        // value-initialize, which means default constructors shall run

        tuple<DefaultConstructor, int> t;

        const DefaultConstructor& dc = get<0>(t);

        REQUIRE(dc.val == DefaultConstructor::default_value());
    }
    SECTION("tuple size")
    {
        using namespace estd;

        SECTION("simple type")
        {
            tuple<bool, bool, bool> t(true, false, true);

            int sz = sizeof(t);

            REQUIRE(sz == 3);
        }
        SECTION("complex empty type")
        {
            const tuple<EmptyClass, EmptyClass, EmptyClass> t;

            int sz = sizeof(t);

            // Now with sparse tuple support, this folds down to truly empty
            REQUIRE(sz == 1);
            REQUIRE(estd::is_empty<decltype(t)>::value);

            // FIX: As it should be, this can't convert a temporary to a reference
            EmptyClass v1 = estd::get<0>(t);

            // DEBT: For empty classes, would be nice to avoid const - though technically
            // const is correct here
            t.visit([](internal::visitor_instance<const EmptyClass> vi)
            {
                return false;
            });
        }
        SECTION("intermixed types")
        {
            tuple<EmptyClass, Dummy, EmptyClass> t;

            int sz = sizeof(t);

            // on debian x64, this comes out to 32
            //REQUIRE(sz == 3);

            // TODO: Look into tuple<T&, TArgs...> see if we accidently light that up
        }
    }
    SECTION("tuple_size")
    {
        SECTION("tuple")
        {
            // TODO: Implement from https://en.cppreference.com/w/cpp/utility/tuple/tuple_size
            auto t = estd::make_tuple(2.0f, 3.0f);
            constexpr int sz = estd::tuple_size<decltype(t)>::value;
            REQUIRE(sz == 2);
        }
        SECTION("pair")
        {
            auto p = estd::make_pair(2.0f, 3.0f);
            constexpr int sz = estd::tuple_size<decltype(p)>::value;
            REQUIRE(sz == 2);
        }
    }
    SECTION("apply")
    {
        // From https://en.cppreference.com/w/cpp/utility/apply

        auto add_lambda = [](int first, int second) { return first + second; };


        SECTION("tuple")
        {
            auto result = estd::apply(add_lambda, estd::make_tuple(2.0f, 3.0f));

            REQUIRE(result == 5);
        }
        SECTION("pair")
        {
            auto result = estd::apply(add_lambda, estd::make_pair(2.0f, 3.0f));

            REQUIRE(result == 5);
        }
        SECTION("parameter pack")
        {
            /*
            auto t1 = estd::make_tuple(2.0f, 3.0f);

            // template argument deduction/substitution failed:
            // couldn’t deduce template parameter ‘F2’
            estd::apply(tester, t1); */
            tester2<float, float> t(2.0, 3.0);

            REQUIRE(t.test() == 5);
        }
    }
    SECTION("converting constructor")
    {
        int counter = 0;
        tuple<test::NonTrivial> t(5);

        REQUIRE(get<0>(t).code_ == 5);

        {
            tuple<test::NonTrivial, int> t2(
                make_tuple(7, [&]{++counter;}),
                77);

            REQUIRE(counter == 0);
            REQUIRE(get<0>(t2).code_ == 7);
            REQUIRE(get<1>(t2) == 77);
        }

        REQUIRE(counter == 1);
    }
    SECTION("visiting")
    {
        SECTION("main")
        {
            // Large overlap with 'apply' but using our own variadic namespace and a bit more powerful
            tuple<test::NonTrivial> v(5);
            int counter = 0;

            // NOTE: Cheating a tiny bit, since we only have 1 item in tuple we don't
            // have to dance around and 'auto' functor.  Only a little bit cheating because
            // this still won't compile or pass if things aren't just right.  Therefore not debt
            v.visit(
                [](variadic::v3::instance<test::NonTrivial> v, int& counter)
                {
                    ++counter;

                    REQUIRE(v.value.copied_ == false);
                    REQUIRE(v.value.moved_ == false);

                    REQUIRE(v.value.code_ == 5);

                    return false;
                }, counter);

            REQUIRE(counter == 1);
        }
        SECTION("empty")
        {
            tuple<> v;

#if __cplusplus >= 201402L
            v.visit([](auto& v)
            {
                FAIL();
            });
#endif
        }
        SECTION("const")
        {
            const tuple<int> v(0);

#if __cplusplus >= 201402L
            int which = v.visit([](const auto& v)
#else
            int which = v.visit([](variadic::v3::instance<const int> v)
#endif
            {
                REQUIRE(v.value == 0);
                return true;
            });

            REQUIRE(which == 0);
        }
        SECTION("array ref")
        {
            static const char (&s)[4] = "hi!";
            const tuple<decltype(s)> v(s);

#if __cplusplus >= 201402L
            int which = v.visit([](const auto& v)
#else
            int which = v.visit([](variadic::v3::instance<decltype(s)> v)
#endif
            {
                REQUIRE(v.value[0] == s[0]);
                REQUIRE(v.value[2] == s[2]);
                return true;
            });

            REQUIRE(which == 0);
        }
    }
    SECTION("sparse vs non-sparse")
    {
        SECTION("sparse")
        {
            internal::tuple<true, monostate, int> t;
            v1::sparse_tuple<monostate, int> t2;

            REQUIRE(sizeof(t) == sizeof(t2));
            REQUIRE(sizeof(t) == sizeof(int));

            auto t3 = make_tuple(monostate{}, 1);

            REQUIRE(sizeof(t3) == sizeof(t));
        }
        SECTION("full (non-sparse)")
        {
            internal::tuple<false, monostate, int> t;

            REQUIRE(sizeof(t) > sizeof(int));
        }
    }
}

#include "macro/pop.h"
