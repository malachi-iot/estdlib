#include <estd/exp/observer.h>
#include <estd/vector.h>

#include <tuple>

#include <catch.hpp>

using namespace estd::experimental;
using namespace estd::experimental::internal;

static int expected;

struct event_1
{
    int data;
};


struct event_2
{
    int data;
};


struct event_3
{
    int data;
};

class StatelessObserver
{
public:
    static void on_notify(int val)
    {
        REQUIRE(val == expected);
    }

    static void on_notify(event_1 val, const int& context)
    {

    }
};

class IObserver
{
public:
    virtual void on_notify(event_1 n) = 0;
};

class FakeBase {};

class StatefulObserver : public FakeBase
{
public:
    int id;

    StatefulObserver() {}

    StatefulObserver(int id) : id(id) {}

    void on_notify(int val)
    {
        REQUIRE(val == expected);
    }

    static void on_notify(event_3 e, event_3& context)
    {
        REQUIRE(e.data == expected);

        context.data = 77;
    }


    void on_notify(event_1 e)
    {
        REQUIRE(e.data == expected);
    }

    void on_notify(const event_2& e)
    {
        REQUIRE(e.data == expected);
    }
};


struct OtherStatefulObserver
{
    char buf[10];

    OtherStatefulObserver()
    {
        buf[0] = 1;
        buf[1] = 2;
        buf[2] = 3;
        buf[3] = 4;
    }

    void on_notify(int val)
    {
        REQUIRE(val == expected);
    }
};


template <class TSubject>
class WithSubject
{
    TSubject subject;

public:
};

StatefulObserver stateful_observer_1, stateful_observer_2;
OtherStatefulObserver stateful_observer_0;

// clang doesn't like constexpr tuple, and to be fair, c++11 doesn't indicate you can make one in
// the spec (gnu allows it)
#if defined(__GNUC__) && !defined(__clang__)
constexpr auto z = std::make_tuple(1, 2);
#endif


TEST_CASE("observer tests")
{
    stateful_observer_1.id = 1;
    stateful_observer_2.id = 2;

    expected = 3;

    SECTION("stateless")
    {
        stateless_subject<StatelessObserver> ss;
        int context = 0;

        ss.notify(3);
        ss.notify(3, context);
        ss.notify(event_1 {}, context);
    }
    SECTION("layer0")
    {
        // probably the best we're gonna do is something like a tuple, which still has a potential memory footprint
        // but for layer0-ish/constant scenarios perhaps more practical than a layer1::vector

        expected = 5;

        /*
        s::notify(3); */
        SECTION("subject")
        {
            auto s = layer0::make_subject(
                    StatefulObserver(0x77),
                    //stateful_observer_0,
                    //stateful_observer_1,
                    stateful_observer_2,
                    OtherStatefulObserver());

            int sz = sizeof(s);
            int sz2 = sizeof(decltype(s));

            REQUIRE(sz2 > 0);

            s.notify(5);
        }
        SECTION("constexpr subject")
        {
            constexpr auto s = layer0::make_subject_const(stateful_observer_0, stateful_observer_1,
                                                          stateful_observer_2);

            // still 24; guess this makes sense, can't constexpr-optimize away instance variables
            int sz = sizeof(s);

            //s.notify(5);
            REQUIRE(sz > 0);
        }
        SECTION("event overloading")
        {
            auto s = layer0::make_subject(stateful_observer_1, stateful_observer_2);

            s.notify(event_1{5});

            expected = 3;

            s.notify(event_2{3});
        }
        SECTION("void event")
        {
            void_subject s;

            // resolves to noop, just in here to make sure it compiles really
            s.notify(event_1{5});
        }
        SECTION("no-constructor version of subject")
        {
            //auto s1 = layer0::make_subject(stateful_observer_1);
            layer0::subject<StatefulObserver> s;

            s.notify(5);
        }
        SECTION("simple proxy test")
        {
            layer0::subject<StatefulObserver> s;
            observer_proxy<decltype(s), int> p(std::move(s));
            auto s2 = layer0::make_subject(p, stateful_observer_1);

            int sz = sizeof(s2);

            REQUIRE(sz > 0);

            s2.notify(5);
        }
        SECTION("make proxy test")
        {
            auto p = make_proxy_2<layer0::subject<StatefulObserver>, observer_abstract<int, event_1> >();
            auto s = layer0::make_subject(p, stateful_observer_1);

            s.notify(5);
        }
#ifdef FEATURE_CPP_DECLTYPE
        SECTION("optional notify test")
        {
            auto s = layer0::make_subject(stateful_observer_1, stateful_observer_2);

            // should resolve to NOOP
            s.notify(event_3 { 5 });
        }
        SECTION("optional context test")
        {
            auto s = layer0::make_subject(stateful_observer_1, stateful_observer_2);
            // for now, context can't be an rvalue
            int context = 0;

            s.notify(5, context);

            event_3 fake_context { 7 };
            s.notify(event_3 { 5 }, fake_context);

            REQUIRE(fake_context.data == 77);
        }
#endif
    }
    SECTION("Lots of stateless")
    {
        stateless_subject<
                StatelessObserver,
                StatelessObserver,
                StatelessObserver> ss;

        ss.notify(3);
    }
    SECTION("container_subject")
    {
        //container_subject<estd::layer1::vector<std::reference_wrapper<IObserver>, 10> > c;
        container_subject<estd::layer1::vector<IObserver*, 10> > c;

        c.notify(event_1 { 3 });
    }
}
