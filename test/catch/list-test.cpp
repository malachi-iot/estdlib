#include <catch.hpp>

#include "estd/forward_list.h"

struct test_value
{

};

struct test_node : public estd::experimental::forward_node_base
{
    int val;
};

TEST_CASE("linkedlist")
{
    SECTION("forward-list")
    {
        estd::forward_list<test_node> list;
        test_node node_a;

        list.push_front(node_a);

        REQUIRE(&list.front() == &node_a);
    }
    SECTION("forward-list 2")
    {
        estd::forward_list<int> list;
        int val = 5;

        //list.push_front(val);
    }
    SECTION("forward-list 3")
    {
        estd::forward_list<test_node> list;
        test_node nodes[3];

        nodes[0].val = 0;
        nodes[1].val = 1;
        nodes[2].val = 2;

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        int c = 0;

        for(const auto& i : list)
        {
            REQUIRE(i.val == c++);
        }

        list.pop_front();

        c = 0;

        REQUIRE(list.front().val == 1);
    }
    SECTION("forward_list insert_after")
    {
        estd::forward_list<test_node> list;
        test_node nodes[3];
        test_node last_node;

        nodes[0].val = 0;
        nodes[1].val = 1;
        nodes[2].val = 2;

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        last_node.val = 3;

        auto i = list.begin();

        i++;
        i++;

        list.insert_after(i, last_node);

        i = list.begin();

        REQUIRE((*i++).val == 0);
        REQUIRE((*i++).val == 1);
        REQUIRE((*i++).val == 2);
        REQUIRE((*i++).val == 3);
    }
}