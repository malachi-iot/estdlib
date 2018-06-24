#include <estd/exp/observer.h>
#include <tuple>

#include <catch.hpp>

using namespace estd::experimental;
using namespace estd::experimental::internal;

class StatelessObserver
{
public:
    static void on_notify(int val)
    {

    }
};

class FakeBase {};

class StatefulObserver : public FakeBase
{
public:
    int id;

    void on_notify(int val)
    {

    }
};


struct OtherStatefulObserver
{
    void on_notify(int val)
    {

    }
};

StatefulObserver stateful_observer_1, stateful_observer_2;
OtherStatefulObserver stateful_observer_0;

constexpr auto z = std::make_tuple(1, 2);


TEST_CASE("observer tests")
{
    stateful_observer_1.id = 1;
    stateful_observer_2.id = 2;

    SECTION("stateless")
    {
        stateless_subject<StatelessObserver> ss;


        ss.notify(3);
    }
    SECTION("layer0")
    {
        // FIX: Very unideal because really this still demands a virtual observer, even though it's all wrapped
        // up with template calls
        typedef layer0::subject<StatefulObserver,
                stateful_observer_1,
                stateful_observer_2> s;

        s::notify(3);

        layer0::test_notify(3, stateful_observer_1, stateful_observer_2);

        // probably the best we're gonna do is something like a tuple, which still has a potential memory footprint
        // but for layer0-ish/constant scenarios perhaps more practical than a layer1::vector

        /*
        s::notify(3); */
        SECTION("subject2")
        {
            auto s = layer0::make_subject(stateful_observer_0, stateful_observer_1, stateful_observer_2);

            s.notify(5);
        }
    }
}