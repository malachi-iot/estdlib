#include <catch.hpp>

#include "estd/forward_list.h"
#include "mem.h"

struct test_value
{
    int val;

    bool operator ==(const test_value& compare_to)
    {
        return compare_to.val == val;
    }
};

struct test_node :
        public estd::experimental::forward_node_base,
        public test_value
{
};


struct test_node_handle_base
{
    uint8_t m_next;

    uint8_t next_node() const { return m_next; }
    void next_node(uint8_t dummy) { m_next = dummy; }

    test_node_handle_base() : m_next(0xFF) {}
};

struct test_node_handle : public test_node_handle_base
{
    int val;

    test_node_handle() {}

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

uint8_t handle_count = 0;
test_node_handle2 handles[5];

template <template <class> class TAllocator>
using node_traits_inlineref = estd::inlineref_node_traits<
        estd::experimental::forward_node_base,
        TAllocator >;

// just to help out typed_handle
// eventually root allocators will shoulder more of the burden that node allocators
// presently do
template <class T>
struct dummy_pool_allocator
{
    typedef uint8_t handle_type;

    // allocator_traits needs this
    typedef T value_type;
    typedef void* pointer;
    typedef const pointer const_void_pointer;
};

struct explicit_handle_node_traits
{
    typedef uint8_t node_handle;
    typedef test_node_handle& nv_reference;

    template <class TValue>
    struct _node_allocator_t
    {
        typedef estd::nothing_allocator<TValue> allocator_t;
        typedef test_node_handle node_type;
        typedef node_type* node_pointer;
        typedef test_node_handle& nv_ref_t;
        // node_allocator_t::node_handle must be identical or more specialized form
        // of node_traits::node_handle
        typedef uint8_t node_handle;

        _node_allocator_t(void*) {}

        static node_handle alloc(TValue& value)
        {
            handles[handle_count] = value;
            return handle_count++;
        }

        // placeholders
        // only useful when a) list is managing node memory allocations and
        // b) when they are handle-based
        node_type& lock(node_handle node) { return handles[node]; }
        void unlock(node_handle node) {}
    };

    typedef test_node_handle node_type;
    //typedef _node_allocator_t::node_type node_type;

    static CONSTEXPR node_handle null_node() { return 0xFF; }

    static node_handle get_next(const node_type& node) { return node.next_node(); }
    static void set_next(node_type& node, node_handle set_to) { node.next_node(set_to); }

    template <class TValue>
    static TValue& value_exp(node_type& node) { return node; }


#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue>
    using node_allocator_t = _node_allocator_t<TValue>;
#endif
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
        estd::forward_list<test_node_handle, explicit_handle_node_traits> list;
        test_node_handle item1;

        item1.val = 7;

        // FIX: temporarily disabled as we revamp (dismantle) node_allocator_t

#ifdef UNUSED
        list.push_front(item1);

        // since the allocator does copies, we can change this
        item1.val = 10;

        // needs some more work before we can do this
        list.insert_after(list.begin(), item1);

        REQUIRE(list.front().val == 7);
        REQUIRE((*++list.begin()).val == 10);
#endif
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
        estd::forward_list<test_value, node_traits_inlineref<_allocator > > list;
        test_value val1;

        val1.val = 3;

        list.push_front(val1);

        auto i = list.begin();

        REQUIRE((*i).val == 3);
        REQUIRE(!list.empty());

        list.pop_front();

        REQUIRE(list.empty());
    }
    SECTION("Forward list: remove")
    {
        estd::forward_list<test_node> list;
        test_node nodes[3];

        nodes[0].val = 0;
        nodes[1].val = 1;
        nodes[2].val = 2;

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        list.remove(nodes[1]);

        auto i = list.begin();

        REQUIRE((*i++).val == 0);
        REQUIRE((*i++).val == 2);
    }
    SECTION("Forward list: remove #2")
    {
        estd::forward_list<estd::experimental::forward_node<int>> list;
        estd::experimental::forward_node<int> nodes[3];

        nodes[0].value() = 0;
        nodes[1].value() = 1;
        new (&nodes[2].value()) int(2);

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        list.remove(nodes[1]);

        auto i = list.begin();

        REQUIRE((*i++).value() == 0);
        REQUIRE((*i++).value() == 2);

        REQUIRE(i == list.end());
    }
    SECTION("Forward list: remove_if")
    {
        typedef estd::experimental::forward_node<int> node_t;
        estd::forward_list<node_t> list;
        node_t nodes[3];

        nodes[0].value() = 0;
        nodes[1].value() = 1;
        new (&nodes[2].value()) int(2);

        list.push_front(nodes[2]);
        list.push_front(nodes[1]);
        list.push_front(nodes[0]);

        list.remove_if([](const node_t& compare_to)
                       { return compare_to.value() == 1; });

        auto i = list.begin();

        REQUIRE((*i++).value() == 0);
        REQUIRE((*i++).value() == 2);
        REQUIRE(i == list.end());
    }
    SECTION("Ensure alloc_front failure for intrusive-style list")
    {
        typedef estd::experimental::forward_node<int> node_t;
        estd::forward_list<node_t> list;

        node_t node1;

        node1.value() = 3;

        list.push_front(node1);
        //list.emplace_front(4);
    }
    SECTION("Forward list: dynamic node allocation, emplacement")
    {
        estd::forward_list<long, node_traits_inlineref<_allocator > > list;

        list.emplace_front(4);
        list.push_front(3);

        auto i = list.begin();

        REQUIRE((*i++) == 3);
        REQUIRE((*i++) == 4);
        REQUIRE(!list.empty());

        list.pop_front();
        list.pop_front();

        REQUIRE(list.empty());
    }
    SECTION("Forward list: dynamic node allocation, inline value")
    {
        estd::forward_list<long, estd::inlinevalue_node_traits<estd::experimental::forward_node_base, _allocator > > list;

        list.emplace_front(4);
        list.push_front(3);

        auto i = list.begin();

        REQUIRE((*i++) == 3);
        REQUIRE((*i++) == 4);
        REQUIRE(!list.empty());

        list.pop_front();
        list.pop_front();

        REQUIRE(list.empty());
    }
}
