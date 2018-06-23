#include <estd/exp/observer.h>

#include <catch.hpp>

using namespace estd::experimental::internal;

class Observer1
{
public:
    static void on_notify(int val)
    {

    }
};


TEST_CASE("observer tests")
{
    SECTION("A")
    {
        stateless_subject<Observer1> ss;


        ss.notify(3);
    }
}