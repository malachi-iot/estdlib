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

class StatefulObserver
{
public:
    void on_notify(int val)
    {

    }
};

StatefulObserver stateful_observer_1;


TEST_CASE("observer tests")
{
    SECTION("stateless")
    {
        stateless_subject<StatelessObserver> ss;


        ss.notify(3);
    }
    SECTION("layer0")
    {
        /*
        typedef layer0::subject<StatefulObserver, stateful_observer_1> s;

        s::notify(3); */
    }
}