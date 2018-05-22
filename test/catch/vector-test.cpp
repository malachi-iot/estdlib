#include <catch.hpp>

#include <estd/vector.h>

#include "mem.h"

using namespace estd;


TEST_CASE("vector tests")
{
    SECTION("basic vector")
    {
        estd::vector<int, _allocator<int>> v;

        v.reserve(20);

        REQUIRE(v.empty());

        auto test = v[3];
        auto& test2 = test.lock();

        test2 = 5;

        REQUIRE(v[3].lock() == 5);
    }
    SECTION("basic vector 2")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(5);

        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() > 5);
        REQUIRE(v[0].lock() == 5);
        REQUIRE(!v.empty());
    }
    SECTION("Vector iterator")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        auto i = v.begin();

        i++;

        REQUIRE(i.lock() == 2);

        i.unlock();

        i++;
        i++;

        REQUIRE(i == v.end());
    }
    SECTION("Vector insert")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(1);
        v.push_back(3);
        v.push_back(4);

        auto i = v.begin();

        i++;

        v.insert(i, 2);

        int counter = 0;

        for(auto i2 : v)
        {
            counter++;
            INFO("Counter: " << counter);
            REQUIRE(counter == i2);
        }

    }
    SECTION("layer1 vector")
    {
        estd::layer1::vector<int, 10> v;
        typedef estd::layer1::vector<int, 10>::accessor accessor_type;

        int size_type_size = sizeof(accessor_type::handle_with_offset::size_type);

        REQUIRE(size_type_size == 1); // not actually used much in this context (but maybe could be?)

        int accessor_size = sizeof(accessor_type);

        // should be relatively small, comprised of an offset and
        // a reference to underlying fixed allocator.  As an
        // optimization, we should be able to fold that down into
        // the stateless pointer-only variety under these scenarios where
        // locking is just a formality to turn a fake handle into
        // a pointer
        REQUIRE(accessor_size <= sizeof(void*)*2);

        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 3);

        accessor_type a = v[0];

        REQUIRE(a == 3);
    }
    SECTION("layer2 vector")
    {
        // Not quite ready, need to de-const the dynamic_array init chain
        /*
        int buf[10];
        estd::layer2::vector<int, 10> v(buf);

        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 3); */
    }
    SECTION("vector with semi-complex type/find")
    {
        struct complex
        {
            int value;
            const char* s;
        };

        estd::layer1::vector<complex, 10> v;

        v.push_back(complex { 3, "hello" });

        auto it = std::find_if(v.begin(), v.end(), [](const complex& value) { return value.value == 3; });

        REQUIRE((*it).value == 3);
        REQUIRE((*it).s == "hello");
    }
    SECTION("vector with actual default std::allocator")
    {
        // eventually will be phased out to use our estd::allocator always
        estd::vector<int> v;

        //v.push_back(3);
    }
    SECTION("vector front/back")
    {
        estd::layer1::vector<int, 10> v;

        v.push_back(5);

        REQUIRE(v.front() == 5);
        REQUIRE(v.back() == 5);
    }
    SECTION("iterator math")
    {
        // In eventual support of using std::priority_queue
        estd::layer1::vector<int, 10> v;
        typedef estd::layer1::vector<int, 10>::iterator iterator;

        v.push_back(1);
        v.push_back(2);

        SECTION("subtraction")
        {

            int diff = v.end() - v.begin();

            REQUIRE(diff == 2);
        }
        SECTION("addition")
        {
            iterator end = v.begin() + 2;

            REQUIRE(v.end() == end);
        }
        SECTION("subtraction again")
        {
            iterator begin = v.end() - 2;

            REQUIRE(v.begin() == begin);
        }
    }
    SECTION("std::make_heap interaction")
    {
        // recreating what is seen
        // http://en.cppreference.com/w/cpp/algorithm/make_heap
        layer1::vector<int, 10> v { 3, 1, 4, 1, 5, 9 };

        REQUIRE(v.size() == 6);

        typedef layer1::vector<int, 10>::iterator iterator;

        std::make_heap(v.begin(), v.end());

        iterator it = v.begin();

        REQUIRE(*it++ == 9);
        REQUIRE(*it++ == 5);
        REQUIRE(*it++ == 4);
        REQUIRE(*it++ == 1);
        REQUIRE(*it++ == 1);
        REQUIRE(*it++ == 3);
    }
}
