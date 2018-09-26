#include <catch.hpp>

#include <estd/stack.h>

TEST_CASE("stack")
{
    SECTION("layer1")
    {
        estd::layer1::stack<int, 10> s;

        s.push(1);
        s.push(2);
        s.push(3);

        int sz = sizeof(s);

        //REQUIRE(sz == sizeof(int) * 10 + sizeof(void*) * 2);

        int counter = 0;

        for(auto& i : s)
        {
            REQUIRE(++counter == i);
        }

        REQUIRE(counter == 3);

        REQUIRE(s.top() == 3);
        s.pop();
        REQUIRE(s.top() == 2);
        s.pop();
        REQUIRE(s.top() == 1);
        s.pop();

    }
}
