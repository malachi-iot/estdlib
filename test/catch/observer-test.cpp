#include <estd/exp/observer.h>

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
    void on_notify(int val)
    {

    }
};

StatefulObserver stateful_observer_1, stateful_observer_2;


TEST_CASE("observer tests")
{
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

        /*
        s::notify(3); */
    }
}