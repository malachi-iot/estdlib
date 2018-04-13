#include <catch.hpp>

#include "estd/forward_list.h"

struct test_value
{
    int val;
};

struct test_node :
        public estd::experimental::forward_node_base,
        public test_value
{
};


struct test_node_handle
{
    uint8_t m_next;

    uint8_t next_node() const { return m_next; }
    void next_node(uint8_t dummy) { m_next = dummy; }

    int val;

    test_node_handle() : m_next(0xFF) {}

    test_node_handle(const test_node_handle& copy_from)
    {
        val = copy_from.val;
    }
};


struct test_node_handle2 : public test_node_handle
{
    int extra_data;

    test_node_handle2(const test_node_handle& copy_from) :
        test_node_handle(copy_from)
    {

    }

    test_node_handle2()
    {

    }
};

int handle_count = 0;
test_node_handle2 handles[5];

class _allocator
{
public:
    // Would like to use this somehow to enable/disable auto-unlocking
    // for iterators
    static constexpr bool is_locking() { return false; }

    typedef estd::nothing_allocator::lock_counter lock_counter;

    typedef void* value_type;
    typedef void* pointer;
    typedef const void* const_void_pointer;
    typedef void* handle_type;

    static handle_type invalid() { return NULLPTR; }

    void* allocate(size_t size)
    {
        return malloc(size);
    }

    void deallocate(void* p, size_t size)
    {
        free(p);
    }
};


// helper traits class for typical, standard pointer based linked list behavior
template <class TValue>
struct node_traits_standard
{

};

// helper traits class for node traits organized like stock-standard std::forward_list
// forward_node_bases are dynamically allocated via TAllocator with an extra space for a TValue&
template <class TValue, class TAllocator>
struct node_traits_inlineref : public node_traits_standard<TValue>
{
    typedef TValue value_type;
    typedef TAllocator allocator_t;
    typedef estd::smart_inlineref_node_alloc<estd::experimental::forward_node_base, value_type, allocator_t> node_allocator_t;
    typedef typename node_allocator_t::node_t node_type;

    typedef const value_type& nv_reference;
    typedef node_type* node_pointer;
    typedef typename allocator_t::handle_type node_handle;

    static CONSTEXPR node_handle null_node() { return allocator_t::invalid(); }

    static node_handle get_next(const node_type& node)
    {
        return node.next();
    }

    static void set_next(node_type& node, node_handle set_to)
    {
        // FIX: this only works because _allocator handle is
        // interchangeable with node_pointer
        node_pointer next = reinterpret_cast<node_pointer>(set_to);
        node.next(next);
    }


    static value_type& value(node_type& node)
    {
        // FIX: clean up this brute-force drop of const
        return (value_type&) node.value;
    }
};

// this will indicate that all test_values used in forward_list
// shall have dynamically allocated node portions
template <>
struct estd::node_traits<test_value> :
        public node_traits_inlineref<test_value, _allocator>
{
};

template <>
struct estd::node_traits<test_node_handle>
{
    typedef test_node_handle node_type;
    typedef node_type value_type;
    typedef uint8_t node_handle;
    typedef test_node_handle& nv_reference;
    typedef test_node_handle* node_pointer;

    static CONSTEXPR node_handle null_node() { return 0xFF; }

    static node_handle get_next(const node_type& node) { return node.next_node(); }
    static void set_next(node_type& node, node_handle set_to) { node.next_node(set_to); }

    static value_type& value(node_type& node) { return node; }

    //typedef smart_inlineref_node_alloc<node_type, value_type, allocator_t> node;

    struct node_allocator_t
    {
        typedef nothing_allocator allocator_t;

        node_allocator_t(void*) {}

        static node_handle alloc(value_type& value)
        {
            handles[handle_count] = value;
            return handle_count++;
        }

        // placeholders
        // only useful when a) list is managing node memory allocations and
        // b) when they are handle-based
        node_pointer lock(node_handle node) { return &handles[node]; }
        void unlock(node_handle node) {}
    };
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
    SECTION("Forward list custom node")
    {
        estd::forward_list<test_node_handle> list;
        test_node_handle item1;

        item1.val = 7;

        list.push_front(item1);

        // since the allocator does copies, we can change this
        item1.val = 10;

        // needs some more work before we can do this
        list.insert_after(list.begin(), item1);

        REQUIRE(list.front().val == 7);
        REQUIRE((*++list.begin()).val == 10);
    }
    SECTION("Forward list erase_after")
    {
        estd::forward_list<test_node> list;
        test_node nodes[3];

        nodes[0].val = 0;
        nodes[1].val = 1;
        nodes[2].val = 2;

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        auto i = list.begin();

        list.erase_after(i);

        i = list.begin();

        REQUIRE((*i++).val == 0);
        REQUIRE((*i++).val == 2);
    }
    SECTION("Forward list: dynamic node allocation, tracking value refs")
    {
        estd::forward_list<test_value> list;
        test_value val1;

        val1.val = 3;

        list.push_front(val1);

        auto i = list.begin();

        REQUIRE((*i).val == 3);
        REQUIRE(!list.empty());

        list.pop_front();

        REQUIRE(list.empty());
    }
}
