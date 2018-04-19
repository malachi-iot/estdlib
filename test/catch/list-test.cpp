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

int handle_count = 0;
test_node_handle2 handles[5];


// helper traits class for node traits organized like stock-standard std::forward_list
// forward_node_bases are dynamically allocated via TAllocator with an extra space for a TValue&
template <class TAllocator>
struct node_traits_inlineref
{
    typedef TAllocator allocator_t;
    typedef estd::experimental::forward_node_base node_type_base;

    // test_node_allocator_t not presently used, trying to decouple node_traits from
    // value_type, if we can
#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue2>
    using test_node_allocator_t = estd::smart_inlineref_node_alloc<
        node_type_base,
        TValue2,
        TAllocator>;
#else
    template <class TValue2>
    struct test_node_allocator_t :
            estd::smart_inlineref_node_alloc<
                node_type_base, TValue2, TAllocator>
    {
        typedef estd::smart_inlineref_node_alloc<node_type_base, TValue2, TAllocator> base_t;

        test_node_allocator_t(TAllocator* allocator) : base_t(allocator) {}
    };
#endif

    typedef node_type_base* node_pointer;
    typedef typename allocator_t::handle_type node_handle;

    static CONSTEXPR node_handle null_node() { return allocator_t::invalid(); }

    static node_handle get_next(const node_type_base& node)
    {
        return node.next();
    }

    static void set_next(node_type_base& node, node_handle set_to)
    {
        // FIX: this only works because _allocator handle is
        // interchangeable with node_pointer
        node_pointer next = reinterpret_cast<node_pointer>(set_to);
        node.next(next);
    }

    // test node allocator base type, use this to extract node_type
    // for value_exp so that we can fully decouple from value_type
    typedef estd::smart_node_alloc<node_type_base, TAllocator> tnab_t;

    template <class TValue2>
    static const TValue2& value_exp(typename tnab_t::template RefNode<TValue2>& node)
    {
        return node.value;
    }
};

struct explicit_handle_node_traits
{
    typedef uint8_t node_handle;
    typedef test_node_handle& nv_reference;
    typedef estd::nothing_allocator allocator_t;

    struct node_allocator_t
    {
        typedef test_node_handle node_type;
        typedef node_type* node_pointer;
        typedef test_node_handle& nv_ref_t;

        node_allocator_t(void*) {}

        template <class TValue>
        static node_handle alloc(TValue& value)
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

    typedef node_allocator_t::node_type node_type;

    static CONSTEXPR node_handle null_node() { return 0xFF; }

    static node_handle get_next(const node_type& node) { return node.next_node(); }
    static void set_next(node_type& node, node_handle set_to) { node.next_node(set_to); }

    template <class TValue>
    static TValue& value_exp(node_type& node) { return node; }


#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue2>
    using test_node_allocator_t = node_allocator_t;
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
    SECTION("Forward list: remove_first")
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
}
