#pragma once

#include "../../platform.h"
#include "../memory.h"

namespace estd {

template <class TNode, class TAllocator = nothing_allocator> struct node_traits;

// trait specifically for extracting value from a node
template <class TNode, class TValue> struct node_value_traits_experimental;



// when linked-list node and tracked value are exactly
// the same
template <class TNode>
struct dummy_node_alloc
{
    typedef TNode node_type;
    typedef node_type* node_pointer;
    typedef node_type& nv_ref_t;

    static CONSTEXPR bool can_emplace() { return false; }

    // pretends to allocate space for a node, when in fact no allocation
    // is necessary for this type
    template <typename TValue>
    node_pointer alloc(TValue& value) { return &value; }

    void dealloc(node_pointer node) {}

    // placeholders
    // only useful when a) list is managing node memory allocations and
    // b) when they are handle-based
    node_pointer lock(node_pointer node) { return node; }
    void unlock(node_pointer node) {}

    dummy_node_alloc(void* allocator) {}
};


// TNode only represents the basic next/reverse tracking portion of the node,
// not the ref or value managed within
template <class TNode, class TAllocator>
class smart_node_alloc
{
protected:
    TAllocator a;

public:
    typedef TAllocator allocator_t;
    typedef allocator_traits<TAllocator> traits_t;
    typedef TNode node_type;
    typedef node_type* node_pointer;
    typedef typename traits_t::handle_type node_handle;

    node_pointer lock(node_handle node)
    {
        return reinterpret_cast<node_pointer>(traits_t::lock(a, node));
    }

    void unlock(node_handle node) { traits_t::unlock(a, node); }

    smart_node_alloc(TAllocator* allocator) :
        a(*allocator) {}

    template <class TValue>
    struct RefNode : TNode
    {
        const TValue& value;

        RefNode(const TValue& value) : value(value) {}
    };


    // ultimately to be used by 'emplace_front'
    template <class TValue>
    struct RefNodeManaged : public RefNode<TValue>
    {
        typedef RefNode<TValue> base_t;

        RefNodeManaged(const TValue& value) :
                base_t(value)
        {}

        ~RefNodeManaged()
        {
            base_t::value.~TValue();
        }
    };




    template <class TValue>
    struct ValueNode : TNode
    {
        const TValue value;

        ValueNode(const TValue& value) : value(value) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
        ValueNode(TValue&& value) : value(value) {}
#endif


#ifdef FEATURE_CPP_VARIADIC
        template <class... TArgs>
        ValueNode(TArgs&&... args) : value(args...) {}
#endif
    };
};


// NOTE: It's possible that in order to implement push_front(&&) and friends,
// we will not only need a smart_inlinevalue_node_alloc, but also be able to
// run-time differenciate between inline-ref and inline-value since the inline
// value we'd be on the hook for allocating and deallocating ourselves (due
// to && representing a temporary variable)
// If there was a way to template-compile-time enforce only one mode and not
// mix and match that might be nice, but so far it only looks #ifdef'able
template <class TNode, class TValue, class TAllocator>
class inlineref_node_alloc : public smart_node_alloc<TNode, TAllocator>
{
    typedef smart_node_alloc<TNode, TAllocator> base_t;
    //typedef node_traits<TNode, TAllocator> node_traits_t;
    typedef typename base_t::traits_t traits_t;

public:
    typedef typename base_t::node_handle node_handle;
    typedef const TValue& nv_ref_t;
    typedef typename base_t::template RefNode<TValue> node_type;
    typedef node_type* node_pointer;

    static CONSTEXPR bool can_emplace() { return true; }

    inlineref_node_alloc(TAllocator* a) :
        base_t(a) {}

    node_handle alloc(const TValue& value)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type));

        void* p = traits_t::lock(this->a, h);

        new (p) node_type(value);

        traits_t::unlock(this->a, h);

        return h;
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    // NOTE: Not sure why overloading doesn't select this properly, but needed to name
    // this alloc_move explicitly
    node_handle alloc_move(TValue&& value)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type) + sizeof(TValue));

        void* p = traits_t::lock(this->a, h);
        void* v = static_cast<uint8_t*>(p) + sizeof(node_type);

        traits_t::construct(this->a, (TValue*)v, value);
        traits_t::construct(this->a, (node_type*)p, *((TValue*)v));

        traits_t::unlock(this->a, h);

        return h;
    }
#endif


#ifdef FEATURE_CPP_VARIADIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    template <class ...TArgs>
    node_handle alloc_emplace( TArgs&&...args)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type) + sizeof(TValue));

        void* p = traits_t::lock(this->a, h);
        void* value = static_cast<uint8_t*>(p) + sizeof(node_type);

        traits_t::construct(this->a, (TValue*)value, args...);
        traits_t::construct(this->a, (node_type*)p, *((TValue*)value));

        traits_t::unlock(this->a, h);

        return h;
    }
#endif

    void dealloc(node_handle h)
    {
        traits_t::deallocate(this->a, h, sizeof(node_type));
    }

    node_pointer lock(node_handle node)
    {
        return reinterpret_cast<node_pointer>(traits_t::lock(this->a, node));
    }

};


template <class TNode, class TValue, class TAllocator>
class inlinevalue_node_alloc : public smart_node_alloc<TNode, TAllocator>
{
    typedef smart_node_alloc<TNode, TAllocator> base_t;
    //typedef node_traits<TNode, TAllocator> node_traits_t;
    typedef typename base_t::traits_t traits_t;

public:
    typedef typename base_t::node_handle node_handle;
    typedef const TValue& nv_ref_t;
    typedef typename base_t::template ValueNode<TValue> node_type;
    typedef node_type* node_pointer;

    static CONSTEXPR bool can_emplace() { return true; }

    inlinevalue_node_alloc(TAllocator* a) :
        base_t(a) {}

    node_handle alloc(const TValue& value)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type));

        void* p = traits_t::lock(this->a, h);

        new (p) node_type(value);

        traits_t::unlock(this->a, h);

        return h;
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    // FIX: had to make it TValue& instead of TValue&&, definitely hacky but at least
    // it's functional
    // NOTE: Not sure why overloading doesn't select this properly, but needed to name
    // this alloc_move explicitly
    node_handle alloc_move(TValue&& value)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type));

        void* p = traits_t::lock(this->a, h);

        traits_t::construct(this->a, (node_type*)p, value);

        traits_t::unlock(this->a, h);

        return h;
    }
#endif


#ifdef FEATURE_CPP_VARIADIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    template <class ...TArgs>
    node_handle alloc_emplace( TArgs&&...args)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type));

        void* p = traits_t::lock(this->a, h);

        traits_t::construct(this->a, (node_type*)p, args...);

        traits_t::unlock(this->a, h);

        return h;
    }
#endif

    void dealloc(node_handle h)
    {
        traits_t::deallocate(this->a, h, sizeof(node_type));
    }

    node_pointer lock(node_handle node)
    {
        return reinterpret_cast<node_pointer>(traits_t::lock(this->a, node));
    }

};

// standardized node traits base.  You don't have to use this, but it proves convenient if you
// adhere to the forward_node_base signature
// FIX: this is hard wired to non-handle based scenarios still
template <class TNode, class TAllocator>
struct node_traits_base
{
    typedef TAllocator allocator_t;
    typedef TNode node_type_base;
    typedef node_type_base* node_pointer;
    typedef typename allocator_t::handle_type node_handle;
    //typedef node_pointer node_handle;

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

    static CONSTEXPR node_handle null_node() { return allocator_t::invalid(); }
};

// helper traits class for node traits organized like stock-standard std::forward_list
// forward_node_bases are dynamically allocated via TAllocator with an extra space for a TValue&
// be advised TNode must conform to forward_node_base signature
template <class TNode, class TAllocator>
struct inlineref_node_traits : public node_traits_base<TNode, TAllocator>
{
    typedef TAllocator allocator_t;
    typedef TNode node_type_base;

    // test_node_allocator_t not presently used, trying to decouple node_traits from
    // value_type, if we can
#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue2>
    using test_node_allocator_t = inlineref_node_alloc<
        node_type_base,
        TValue2,
        TAllocator>;
#else
    template <class TValue2>
    struct test_node_allocator_t :
            inlineref_node_alloc<
                node_type_base, TValue2, TAllocator>
    {
        typedef estd::smart_inlineref_node_alloc<node_type_base, TValue2, TAllocator> base_t;

        test_node_allocator_t(TAllocator* allocator) : base_t(allocator) {}
    };
#endif

    // test node allocator base type, use this to extract node_type
    // for value_exp so that we can fully decouple from value_type
    typedef estd::smart_node_alloc<node_type_base, TAllocator> tnab_t;

    template <class TValue2>
    static const TValue2& value_exp(typename tnab_t::template RefNode<TValue2>& node)
    {
        return node.value;
    }
};



template <class TNode, class TAllocator>
struct inlinevalue_node_traits : public node_traits_base<TNode, TAllocator>
{
    typedef TAllocator allocator_t;
    typedef TNode node_type_base;

    // test_node_allocator_t not presently used, trying to decouple node_traits from
    // value_type, if we can
#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue2>
    using test_node_allocator_t = inlinevalue_node_alloc<
        node_type_base,
        TValue2,
        TAllocator>;
#else
    template <class TValue2>
    struct test_node_allocator_t :
            inlinevalue_node_alloc<
                node_type_base, TValue2, TAllocator>
    {
        typedef estd::smart_inlineref_node_alloc<node_type_base, TValue2, TAllocator> base_t;

        test_node_allocator_t(TAllocator* allocator) : base_t(allocator) {}
    };
#endif

    // test node allocator base type, use this to extract node_type
    // for value_exp so that we can fully decouple from value_type
    typedef estd::smart_node_alloc<node_type_base, TAllocator> tnab_t;

    template <class TValue2>
    static const TValue2& value_exp(typename tnab_t::template RefNode<TValue2>& node)
    {
        return node.value;
    }
};

// this is where node and value are combined, and no allocator is used
// (node memory management entirely external to node and list)
template<class TNodeAndValue>
struct intrusive_node_traits : public node_traits_base<TNodeAndValue, nothing_allocator>
{
    typedef TNodeAndValue node_type;

    // TODO: eventually interact with allocator for this (in
    // other node_traits where allocation actually happens)
    typedef node_type* node_pointer;
    typedef node_pointer node_handle;

    typedef nothing_allocator allocator_t;

    static CONSTEXPR node_pointer null_node() { return NULLPTR; }

    // semi-experimental, since std forward list technically supports a 'before begin'
    // iterator, we may need a before_begin_node() value
    static CONSTEXPR node_pointer after_end_node_experimental() { return NULLPTR; }

    static node_pointer get_next(const node_type& node)
    {
        // NOTE: we assume that node_type represents a very specific type derived ultimately
        // from something resembling forward_node_base, specifically in that
        // a call to next() shall return a pointer to the next node_type*
        return reinterpret_cast<node_pointer>(node.next());
    }

    static node_pointer get_prev(const node_type& node)
    {
        return reinterpret_cast<node_pointer>(node.prev());
    }

    static void set_next(node_type& node, node_handle set_to)
    {
        node.next(set_to);
    }

    template <class TValue2>
    static TValue2& value_exp(node_type& node) { return node; }

    // replacement for old allocator get associated value
    //static value_type& value(node_type& node) { return node; }

    // instance portion which deals with pushing and pulling things in
    // and out of handle & allocations.  Use as an instance even if
    // no instance variables here - should optimize out to static-like
    // assembly if indeed
    // no instance variables present
    // eventually our formalized allocator might be able to displace this
    typedef dummy_node_alloc<node_type> node_allocator_t;

#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue2>
    using test_node_allocator_t = dummy_node_alloc<node_type>;
#endif

};


}
