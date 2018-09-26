#include <catch.hpp>

#include <estd/stack.h>

TEST_CASE("stack")
{
    SECTION("layer1")
    {
        estd::layer1::stack<int, 10> s;

        typedef decltype (s)::size_type size_type;
        typedef decltype (s)::container_type container_type;

        s.push(1);
        s.push(2);
        s.push(3);

        int sz = sizeof(s);

        REQUIRE(sizeof(size_type) == 1);
        // FIX: Want this to come out 41, clearly alignment and padding are at play
        // this is not a specific failure, but kind of a 'loose screw' which I'd rather
        // not completely fall out and cause other problems
        REQUIRE(sizeof(container_type) == 44);
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
