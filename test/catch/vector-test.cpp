#include <catch.hpp>

#include <estd/vector.h>
#include <estd/functional.h>

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

        SECTION("erasing")
        {
            v.erase(v.begin() + 1);

            REQUIRE(v.size() == 3);
            REQUIRE(v[0] == 1);
            REQUIRE(v[1] == 3);
        }
    }
    SECTION("layer1 vector")
    {
        typedef estd::layer1::vector<int, 10> vector_type;
        typedef vector_type::accessor accessor_type;
        typedef vector_type::size_type size_type;
        vector_type v;

        REQUIRE(sizeof(vector_type) == (sizeof(int) * 10) + (sizeof(size_type)));

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

        // Push it to the very limit
        v.clear();

        for(int i = 0; i < 10; ++i)
            v.push_back(i);
    }
    SECTION("layer2 vector")
    {
        int buf[10];
        estd::layer2::vector<int, 10> v(buf);

        // FIX: it's tracking explicit handle right now, but shouldn't -
        // specialization should have eliminated that.  Wait until we've
        // cleaned out null_termination from allocators to fix this
        //REQUIRE(sizeof(v) == sizeof(int*) + sizeof(size_t));
        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 3);
    }
    SECTION("layer3 vector")
    {
        int buf[10];
        estd::layer3::vector<int> v(buf);

        REQUIRE(sizeof(v) == sizeof(int*) + sizeof(size_t) + sizeof(size_t));
        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() == 10);
        REQUIRE(v[0] == 3);
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
    SECTION("Emplace at back")
    {
        layer1::vector<int, 10> v;

        v.emplace_back(5);

        REQUIRE(v.size() == 1);
    }
    SECTION("std::move testing")
    {
        struct Item
        {
            int val;

            Item() : val(0) {}

            Item(Item&& move_from) :
                val(move_from.val)
            {
                move_from.val = -1;
            }
        };

        layer1::vector<Item, 10> v;

        Item i, i2;

        v.push_back(std::move(i));

        i2.val = 5;

        v.insert(v.end(), std::move(i2));

        auto it = v.begin();

        REQUIRE((*it++).val == 0);
        REQUIRE((*it++).val == 5);
        REQUIRE(it == v.end());

        SECTION("empty() call testing")
        {
            REQUIRE(!v.empty());
        }
    }
    SECTION("ref wrapper")
    {
        //layer1::vector<reference_wrapper<int>, 10> a;
    }
    SECTION("experimental")
    {
        SECTION("vector of functions")
        {
            int value = 0;

            layer1::vector<estd::detail::function<void(int)>, 10> v;

            estd::experimental::function<void(int)> f([&](int v)
            {
                value += v;
            });

            v.emplace_back(f);
            v.emplace_back(f);

            for(auto _f : v)
                _f(5);

            REQUIRE(value == 10);
        }
    }
}
