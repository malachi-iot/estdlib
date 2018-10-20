#pragma once

#include "../internal/platform.h"
#include "../memory.h"

namespace estd {

// TODO: Utilize rebind and switch this to just one allocator.  First, do some proof of concept
// rebind testing in playground.gcc
template <class TNode,
          class TNodeAllocator,
          // TValueAllocator *should* actually be for TNode::value_type but code is in
          // flux so not committing to that yet, and as long as it compiles, nothing_allocator
          // should be harmless anyway
          class TValueAllocator >
struct node_traits;

namespace experimental {

template <class TValue, class TNodeBase = int>
class RefNode : public TNodeBase
{
protected:
    TValue& m_value;

public:
    typedef TValue value_type;

#ifdef FEATURE_CPP_MOVESEMANTIC
    RefNode(value_type&& value) : m_value(std::move(value)) {}
#endif

    RefNode(const value_type& value) : m_value(value) {}

    value_type& value() { return m_value; }
};


template <class TValue, class TNodeBase = int>
class ValueNode : public TNodeBase
{
protected:
    TValue m_value;

public:
    typedef TValue value_type;

    ValueNode(const value_type& value) : m_value(value) {}

    // it's basically gaurunteed that movesemantic comes with variadic feature, but
    // the code was already written to switch out
#ifdef FEATURE_CPP_MOVESEMANTIC
#ifdef FEATURE_CPP_VARIADIC
    template <class ...TArgs>
    ValueNode(TArgs&&...args) : m_value(std::forward<TArgs>(args)...)
    {

    }
#else
    ValueNode(value_type&& value) : m_value(std::move(value)) {}
#endif
#endif

    value_type& value() { return m_value; }

    const value_type& value() const { return m_value; }
};


// idea capture for later, explicitly state what variety of node to use and
// then we can do allocator/node_traits specialization on full TNode itself
// (think internal::ValueNode<TValue> or similar).  We should be able to bring it
// all back home to node_trait for all allocations etc. and now that we have
// a *full* TNode, we'll have implicit access to TValue also
//
// Minor problem is that TNode next() decltype must match exactly with TAllocator handle_type.  Not a huge issue
// but easy to get wrong
// TNode = TValue means intrusive node
template <class TValue,
          class TNode = TValue,
          //class TNode = ValueNode<TValue>,
          class TAllocator = experimental_std_allocator<TNode>,
          class TTraits = node_traits<TNode, TAllocator, nothing_allocator< TValue > >
          >
class list;



}


// assumes TNode has a next() and next(node)
template <class TNode, class TNodeAllocator, class TValueAllocator>
struct node_traits_new_base
{
    typedef TNode node_type;
    typedef TNodeAllocator node_allocator_type;
    typedef typename node_allocator_type::handle_type node_handle;
    typedef TValueAllocator value_allocator_type;
    typedef typename value_allocator_type::value_type value_type;

    // TODO: assert value_allocator_type = value_type

    static node_handle CONSTEXPR eol() { return node_allocator_type::invalid(); }

    node_handle next(node_type& node) const
    {
        // NOTE: I don't like doing a cast here, but it is pretty standard stuff
        // for linked lists.  Note that this *should* work properly even with true
        // handle-based lists
        return static_cast<node_handle>(node.next());
    }

    node_handle prev(node_type& node) const
    {
        return static_cast<node_handle>(node.prev());
    }

    void next(node_type &node, const node_handle& new_next)
    {
        node.next(new_next);
    }

    void prev(node_type &node, const node_handle& new_prev)
    {
        node.prev(new_prev);
    }

    // helper call.  May not be the way *you* acquire the tracked value for this node, but
    // used frequently internally
    static value_type& value(node_type& n)
    {
        return n.value();
    }
};


// FIX: I think this is kind of a no no, traits are supposed to be stateless I think
// so probably rename this chain to 'list_helper" or similar like dynamic_array, though
// I don't like *that* name either
template <class TNode, class TAllocator, class TValueAllocator>
struct stateful_allocator_node_traits_base
        : public node_traits_new_base<TNode, TAllocator, TValueAllocator>
{
    typedef node_traits_new_base<TNode, TAllocator, TValueAllocator> base_t;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef typename base_t::node_handle node_handle;
    typedef typename base_t::node_type node_type;
    typedef typename TNode::value_type value_type;

protected:
    // TODO: resolve 'empty' / stateless allocators
    TAllocator node_allocator;
    // NOTE: value_allocator should never be used for inlinevalue, just here
    // for decoration
    TValueAllocator value_allocator;

    node_handle allocate_node()
    {
        return allocator_traits::allocate(node_allocator, 1);
    }

    node_type& lock_node(node_handle h)
    {
        return allocator_traits::lock(node_allocator, h);
    }

    void unlock_node(node_handle h)
    {
        allocator_traits::unlock(node_allocator, h);
    }

    void deallocate_node(node_handle h)
    {
        allocator_traits::deallocate(node_allocator, h, 1);
    }


    void destruct_node(node_handle h)
    {
        // explicit destruction of our node
        (&lock_node(h))->~node_type();
        unlock_node(h);
    }

public:
    // call destructor on and deallocate node
    void destroy(node_handle node_handle)
    {
        destruct_node(node_handle);
        deallocate_node(node_handle);
    }

    TAllocator& get_node_allocate() { return node_allocator; }
};


template <class TNode, class TAllocator, class TValueAllocator>
struct stateless_allocator_node_traits_base
        : public node_traits_new_base<TNode, TAllocator, TValueAllocator>
{
    typedef node_traits_new_base<TNode, TAllocator, TValueAllocator> base_t;
    typedef typename base_t::node_handle node_handle;
    typedef typename base_t::node_type node_type;

protected:
    typedef typename base_t::node_allocator_type allocator_type;

    allocator_type get_node_allocate() { return allocator_type(); }
};

// assumes TNode has a next() and next(node), as
// well as a value_type typedef clue and a constructor which
// takes value_type& for initialization.  Being inline, it's
// likely we favor emplace if it's available to avoid copying
template <class TNode, class TAllocator, class TValueAllocator>
struct inlinevalue_node_traits_new_base :
        public stateful_allocator_node_traits_base<TNode, TAllocator, TValueAllocator>
{
    typedef stateful_allocator_node_traits_base<TNode, TAllocator, TValueAllocator> base_t;
    typedef typename TNode::value_type value_type;
    typedef typename base_t::node_allocator_type node_allocator_type;
    typedef typename base_t::node_handle node_handle;
    typedef typename base_t::node_type node_type;
    typedef typename base_t::allocator_traits allocator_traits;

    // Though value and node is combined at a low level,
    // nv_ref_t 'ideally' represents const value_type& and in this case can
    typedef const value_type& nv_ref_t;

    static CONSTEXPR bool can_emplace()
    {
#ifdef FEATURE_CPP_VARIADIC
        return true;
#else
        return false;
#endif
    }

    node_type& lock(node_handle& h)
    {
        return base_t::lock_node(h);
    }


    void unlock(node_handle& h)
    {
        base_t::unlock_node(h);
    }

    // remember inlinevalue we specifically DO NOT use TValueAllocator

    // standard copy version
    node_handle allocate(const value_type& value)
    {
        node_handle h = base_t::allocate_node();

        node_type& n = base_t::lock_node(h);

        new (&n) node_type(value);

        base_t::unlock_node(h);

        return h;
    }

    void deallocate(node_handle& h)
    {
        base_t::deallocate_node(h);
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class... TArgs>
    node_handle alloc_emplace(TArgs&&...args)
    {
        node_handle h = base_t::allocate_node();

        node_type& n = base_t::lock_node(h);

        // TODO: use allocator_traits construct
        new (&n) node_type(std::forward<TArgs>(args)...);

        base_t::unlock_node(h);

        return h;
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    node_handle alloc_move(value_type&& v)
    {
        node_handle h = base_t::allocate_node();

        node_type& n = base_t::lock_node(h);

        // expects constructor to pass through the moved value
        // (in other words, node_type has a value_type&& constructor)
        new (&n) node_type(std::move(v));

        base_t::unlock_node(h);

        return h;
    }
#endif
};


// assumes TNode has a next() and next(node)
// also being intrusive, TNode must BE value_type;
template <class TNode, class TAllocator, class TValueAllocator>
struct intrusive_node_traits_new_base :
        public stateless_allocator_node_traits_base<TNode, TAllocator, TValueAllocator>
{
    typedef stateless_allocator_node_traits_base<TNode, TAllocator, TValueAllocator> base_t;
    typedef TNode value_type;
    typedef value_type node_type;

    // brute force node_handle as value_type* here because we do no allocations at all, so
    // we want NULLPTR to be a possible value to detect eol
    typedef value_type* node_handle;

    // combined value and node, so nv_ref_t is TNode
    typedef TNode& nv_ref_t;

    // neither TAllocator nor TValueAllocator is directly utilized in this context
    // (intrusive nodes are externally allocated)
    // however, TAllocator is used to at least get the handle/pointer types

    // "allocate" a node to accomodate value type.  However, intrusive nodes ARE value_type,
    // so this is a noop [zero allocation happens in these scenarios]
    node_handle allocate(value_type& value)
    {
        return &value;
    }

    value_type& lock(node_handle value)
    {
        return *value;
    }

    void unlock(node_handle&) {}


    void deallocate(node_handle&) {}

    // again, TNode == TValue here
    static nv_ref_t value(node_type& n)
    {
        return n;
    }

    node_handle next(node_type& node) const
    {
        // Mostly safe to forward cast from intrusive node, typical type of cast operation
        // for using linked list libs
        return static_cast<node_handle>(node.next());
    }

    node_handle prev(node_type& node) const
    {
        // Mostly safe to forward cast from intrusive node, typical type of cast operation
        // for using linked list libs
        return static_cast<node_handle>(node.prev());
    }

    void next(node_type &node, const node_handle& new_next)
    {
        node.next(new_next);
    }

};


#ifdef UNUSED
// when linked-list node and tracked value are exactly
// the same
template <class TNode>
struct dummy_node_alloc
{
    typedef TNode node_type;
    typedef node_type* node_pointer;
    typedef node_type& nv_ref_t;
    typedef node_pointer node_handle;

    typedef nothing_allocator<TNode> allocator_t;

    static CONSTEXPR bool can_emplace() { return false; }

    // pretends to allocate space for a node, when in fact no allocation
    // is necessary for this type
    template <typename TValue>
    node_pointer alloc(TValue& value) { return &value; }

    void dealloc(node_pointer node) {}

    dummy_node_alloc(void* allocator) {}
};


namespace internal {

template <class TValue, class TNodeBase>
struct RefNode : public TNodeBase
{
    const TValue& value;

    RefNode(const TValue& value) : value(value) {}
};


// ultimately to be used by 'emplace_front'
template <class TValue, class TNodeBase>
struct RefNodeManaged : public RefNode<TValue, TNodeBase>
{
    typedef RefNode<TValue, TNodeBase> base_t;

    RefNodeManaged(const TValue& value) :
            base_t(value)
    {}

    ~RefNodeManaged()
    {
        base_t::value.~TValue();
    }
};




template <class TValue, class TNodeBase>
struct ValueNode : public TNodeBase
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

}


// TNode only represents the basic next/reverse tracking portion of the node,
// not the ref or value managed within
template <class TAllocator>
class smart_node_alloc
{
public:
    typedef TAllocator allocator_t;

protected:
    allocator_t a;

public:
    typedef allocator_traits<allocator_t> traits_t;
    typedef typename traits_t::value_type node_type;
    typedef node_type* node_pointer;
    typedef typename traits_t::size_type size_type;
    typedef typename traits_t::handle_type node_handle;

    smart_node_alloc(allocator_t* allocator) :
        a(*allocator) {}

protected:
    node_handle allocate(size_type count = 1)
    {
        return traits_t::allocate(a, count);
    }

    node_type& lock(node_handle h)
    {
        return traits_t::lock(a, h);
    }
};

// NOTE: It's possible that in order to implement push_front(&&) and friends,
// we will not only need a smart_inlinevalue_node_alloc, but also be able to
// run-time differenciate between inline-ref and inline-value since the inline
// value we'd be on the hook for allocating and deallocating ourselves (due
// to && representing a temporary variable)
// If there was a way to template-compile-time enforce only one mode and not
// mix and match that might be nice, but so far it only looks #ifdef'able
template <class TNodeBase,
            class TValue,
            template <class> class TNodeAllocator,
            template <class> class TValueAllocator = TNodeAllocator
            >
class inlineref_node_alloc :
        public smart_node_alloc
            <TNodeAllocator
                <internal::RefNode<TValue, TNodeBase> >
            >
{
public:

private:
    typedef smart_node_alloc<TNodeAllocator <internal::RefNode<TValue, TNodeBase> > > base_t;
    //typedef node_traits<TNode, TAllocator> node_traits_t;
    typedef typename base_t::traits_t traits_t;

public:
    typedef typename base_t::allocator_t allocator_t;
    typedef TValueAllocator<TValue> value_allocator_t;
    typedef typename base_t::node_type node_type;
    typedef typename base_t::node_handle node_handle;
    typedef const TValue& nv_ref_t;
    typedef node_type* node_pointer;

    static CONSTEXPR bool can_emplace() { return true; }

    inlineref_node_alloc(allocator_t* a) :
        base_t(a) {}

    node_handle alloc(const TValue& value)
    {
        node_handle h = base_t::allocate();

        node_type& p = base_t::lock(h);

        new (&p) node_type(value);

        traits_t::unlock(this->a, h);

        return h;
    }


#ifdef FEATURE_CPP_MOVESEMANTIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    // NOTE: Not sure why overloading doesn't select this properly, but needed to name
    // this alloc_move explicitly
    node_handle alloc_move(TValue&& value_to_move)
    {
        allocator_t& a = this->a;

        // we can use typed_handle here because we lead with node_type
        int hack_oversize = 1 + (sizeof(TValue) / sizeof(node_type));
        node_handle h = traits_t::allocate(this->a, 1 + hack_oversize);
        //node_handle h = traits_t::allocate(a, sizeof(node_type) + sizeof(TValue));

        node_type& p = traits_t::lock(a, h);
        TValue* v = (TValue*)(&p + 1);

        traits_t::construct(a, v, value_to_move);
        traits_t::construct(a, &p, *v);

        traits_t::unlock(a, h);

        return h;
    }
#endif


#ifdef FEATURE_CPP_VARIADIC
    // FIX: Still doesn't know to call ~TValue, though it does implicitly deallocate
    // its memory
    template <class ...TArgs>
    node_handle alloc_emplace( TArgs&&...args)
    {
        // we can use typed_handle here because we lead with node_type
        // +++
        // FIX: here we have a huge problem, we can't do byte by byte allocation easily when following
        // TAllocate<T> convention.  However, having TAllocate<T> is so convenient, perhaps deviating
        // and making an "malloc/free" (byte-oriented only) is reasonable - and possibly also convenient
        // because things like pool allocators could just not implement those
        //node_handle h = traits_t::allocate(this->a, sizeof(node_type) + sizeof(TValue));
        int hack_oversize = 1 + (sizeof(TValue) / sizeof(node_type));
        node_handle h = traits_t::allocate(this->a, 1 + hack_oversize);
        // ---

        node_type& p = traits_t::lock(this->a, h);
        void* value = reinterpret_cast<uint8_t*>(&p) + sizeof(node_type);

        traits_t::construct(this->a, (TValue*)value, args...);
        traits_t::construct(this->a, &p, *((TValue*)value));

        traits_t::unlock(this->a, h);

        return h;
    }
#endif

    // FIX: this dealloc will need more attention since different styles of above alloc
    // will require different deallocation techniques
    void dealloc(node_handle& node)
    {
        traits_t::deallocate(this->a, node, 1);
    }
};


template <class TNodeBase, class TValue, template <class> class TAllocator>
class inlinevalue_node_alloc :
        public smart_node_alloc<TAllocator < internal::ValueNode<TValue, TNodeBase > > >
{
    typedef smart_node_alloc<TAllocator < internal::ValueNode<TValue, TNodeBase > > > base_t;
    //typedef node_traits<TNode, TAllocator> node_traits_t;
    typedef typename base_t::traits_t traits_t;

public:
    typedef typename base_t::allocator_t allocator_t;
    typedef typename base_t::node_handle node_handle;
    typedef const TValue& nv_ref_t;
    typedef typename base_t::node_type node_type;
    typedef node_type* node_pointer;

    static CONSTEXPR bool can_emplace() { return true; }

    inlinevalue_node_alloc(allocator_t* a) :
        base_t(a) {}

    node_handle alloc(const TValue& value)
    {
        node_handle h = base_t::allocate();

        node_type& p = base_t::lock(h);

        new (&p) node_type(value);

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
        node_handle h = base_t::allocate();

        node_type& p = base_t::lock(h);

        traits_t::construct(this->a, &p, value);

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
        node_handle h = base_t::allocate();

        node_type& p = base_t::lock(h);

        traits_t::construct(this->a, &p, args...);

        traits_t::unlock(this->a, h);

        return h;
    }
#endif

    void dealloc(node_handle h)
    {
        traits_t::deallocate(this->a, h, 1);
    }
};

// standardized node traits base.  You don't have to use this, but it proves convenient if you
// adhere to the forward_node_base signature
// FIX: this is hard wired to non-handle based scenarios still
template <class TNodeBase, template <class> class TAllocator>
struct node_traits_base
{
    typedef TNodeBase node_type_base;
    typedef node_type_base* node_pointer;

    // we expect node_handle and invalid() to not deviate between TAllocator<node_type_base>
    // and TAllocator<node_type>, though theoretically it could
    typedef typename TAllocator<node_type_base>::handle_type node_handle;

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

    static CONSTEXPR node_handle null_node() { return TAllocator<node_type_base>::invalid(); }
};

// helper traits class for node traits organized like stock-standard std::forward_list
// forward_node_bases are dynamically allocated via TAllocator with an extra space for a TValue&
// be advised TNode must conform to forward_node_base signature
template <class TNodeBase, template <class> class TAllocator>
struct inlineref_node_traits : public node_traits_base<TNodeBase, TAllocator>
{
    typedef node_traits_base<TNodeBase, TAllocator> base_t;
    typedef TNodeBase node_type_base;

#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue>
    using node_allocator_t = inlineref_node_alloc<
        node_type_base,
        TValue,
        TAllocator>;
#else
    template <class TValue2>
    struct node_allocator_t :
            inlineref_node_alloc<
                node_type_base, TValue2, TAllocator>
    {
        typedef estd::inlineref_node_alloc<node_type_base, TValue2, TAllocator> base_t;

        node_allocator_t(typename base_t::allocator_t* allocator) : base_t(allocator) {}
    };
#endif

    template <class TValue>
    static const TValue& value_exp(internal::RefNode<TValue, TNodeBase>& node)
    {
        return node.value;
    }
};



template <class TNodeBase, template <class> class TAllocator>
struct inlinevalue_node_traits : public node_traits_base<TNodeBase, TAllocator>
{
    typedef node_traits_base<TNodeBase, TAllocator> base_t;
    typedef TNodeBase node_type_base;

#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue>
    using node_allocator_t = inlinevalue_node_alloc<
        node_type_base,
        TValue,
        TAllocator>;
#else
    template <class TValue2>
    struct node_allocator_t :
            inlinevalue_node_alloc<
                node_type_base, TValue2, TAllocator>
    {
        typedef estd::inlinevalue_node_alloc<node_type_base, TValue2, TAllocator> base_t;

        node_allocator_t(typename base_t::allocator_t* allocator) : base_t(allocator) {}
    };
#endif

    template <class TValue>
    static const TValue& value_exp(internal::ValueNode<TValue, TNodeBase>& node)
    {
        return node.value;
    }
};

// this is where node and value are combined, and no allocator is used
// (node memory management entirely external to node and list)
template<class TNodeAndValue>
struct intrusive_node_traits : public node_traits_base<TNodeAndValue, nothing_allocator >
{
    typedef node_traits_base<TNodeAndValue, nothing_allocator> base_t;
    typedef TNodeAndValue node_type;

    // TODO: eventually interact with allocator for this (in
    // other node_traits where allocation actually happens)
    typedef node_type* node_pointer;
    typedef node_pointer node_handle;

    static CONSTEXPR node_handle null_node() { return NULLPTR; }

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

#ifdef FEATURE_CPP_ALIASTEMPLATE
    template <class TValue>
    using node_allocator_t = dummy_node_alloc<node_type>;
#endif

};
#endif



// specializes on TNode == TValue by saying if:
// a) TValueAllocator == nothing_allocator NODE
// b) node_traits<TNode matching on node_traits<TValue
// then we assume intrusive.  Feels kinda fragile
template <class TNode, class TValue, class TNodeAllocator>
struct node_traits<TValue, TNodeAllocator, nothing_allocator<TNode> > :
        public intrusive_node_traits_new_base<TNode,
            nothing_allocator<TNode>,
            nothing_allocator<TNode> >
{

};


}
