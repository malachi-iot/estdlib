#pragma once

#include "../platform.h"
#include "memory.h"

namespace estd {

template <class TNode, class TAllocator = nothing_allocator> struct node_traits;

// trait specifically for extracting value from a node
template <class TNode, class TValue> struct node_value_traits_experimental;

namespace experimental {
// Make list and forward list conform to, but not necessarily depend on, these
// signatures (C++ concepts would be nice here)

class forward_node_base
{
    forward_node_base *m_next;

public:
    forward_node_base() : m_next(NULLPTR) {}

    forward_node_base* next() const { return m_next; }

    void next(forward_node_base *set_to) { m_next = set_to; }
};

class reverse_node_base
{
    reverse_node_base* m_prev;

public:
    reverse_node_base() : m_prev(NULLPTR) {}

    reverse_node_base* prev() const { return m_prev; }

    void prev(reverse_node_base* set_to) { m_prev = set_to; }
};


class double_node_base :
        public forward_node_base,
        public reverse_node_base
{};

template<class T>
class forward_node :
        public forward_node_base {
    T m_value;

public:
    T& value() { return m_value; }

    const T& value() const { return m_value; }

    const forward_node<T> *next() const {
        return (forward_node<T> *) forward_node_base::next();
    }

    void next(forward_node<T> *set_to) {
        forward_node_base::next(set_to);
    }

    bool operator==(const forward_node& compare_to) const
    {
        return m_value == compare_to.value();
    }
};

template<class T>
class double_node :
        public forward_node_base,
        public reverse_node_base {
    T m_value;

public:
    T &value() { return m_value; }
};


}

/*
template <class TForwardNodeValue, class TFowardNode = experimental::forward_node<TForwardNodeValue > >
struct node_traits
{
    typedef TForwardNodeValue value_type;
    typedef TFowardNode node_type;

    template <class TList>
    static node_type& front(const TList& list) { return list.front(); }

    static const node_type* get_next(const node_type& node) { return node.next(); }
    static void set_next(node_type& node, node_type* set_to) { node.next(set_to); }
    static value_type& value(node_type& node) { return node.value(); }
};
*/


// when linked-list node and tracked value are exactly
// the same
template <class TNode>
struct dummy_node_alloc
{
    typedef TNode node_type;
    typedef node_type* node_pointer;
    typedef node_type& nv_ref_t;
    typedef nothing_allocator allocator_t;

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
    TAllocator& a;

public:
    typedef TAllocator allocator_t;
    typedef allocator_traits<TAllocator> traits_t;
    //typedef node_traits<TNode> node_traits_t;
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

    template <class TValue>
    struct ValueNode : TNode
    {
        const TValue value;

        ValueNode(const TValue& value) : value(value) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
        ValueNode(TValue&& value) : value(value) {}
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
class smart_inlineref_node_alloc : public smart_node_alloc<TNode, TAllocator>
{
    typedef smart_node_alloc<TNode, TAllocator> base_t;
    //typedef node_traits<TNode, TAllocator> node_traits_t;
    typedef typename base_t::traits_t traits_t;

public:
    typedef typename base_t::node_handle node_handle;
    typedef const TValue& nv_ref_t;
    typedef typename base_t::template RefNode<TValue> node_type;
    typedef node_type* node_pointer;

    smart_inlineref_node_alloc(TAllocator* a) :
        base_t(a) {}

    node_handle alloc(const TValue& value)
    {
        node_handle h = traits_t::allocate(this->a, sizeof(node_type));

        void* p = traits_t::lock(this->a, h);

        new (p) node_type(value);

        traits_t::unlock(this->a, h);

        return h;
    }


    void dealloc(node_handle h)
    {
        traits_t::deallocate(this->a, h, sizeof(node_type));
    }

    node_pointer lock(node_handle node)
    {
        return reinterpret_cast<node_pointer>(traits_t::lock(this->a, node));
    }

};

/*
template <class TNode, class TValue, class TAllocator>
struct node_value_traits_experimental<
        typename smart_node_alloc<TNode, TAllocator>::template RefNode<TValue>, TValue>
{

    //const TValue& value()
};
*/

// this is where node and value are combined, and no allocator is used
// (node memory management entirely external to node and list)
template<class TNode>
struct node_traits_noalloc
{
    typedef TNode node_type;

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

// default node_traits is the no-alloc variety (since we are embedded oriented)
template <class TValue, class TAllocator>
struct node_traits : public node_traits_noalloc<TValue> {};


// adapted from util.embedded version
template <class TValue, class TNodeTraits>
struct InputIterator
{
    typedef TNodeTraits traits_t;
    typedef TValue value_type;
    typedef typename traits_t::template test_node_allocator_t<value_type> node_allocator_t;
    typedef typename TNodeTraits::node_handle node_handle_t;
    typedef InputIterator<TValue, TNodeTraits> iterator;
    typedef const iterator const_iterator;

protected:
    node_handle_t current;

    //typedef typename traits_t::node_allocator_t node_alloc_t;
    typedef typename node_allocator_t::node_type node_type;
    typedef typename node_allocator_t::node_pointer node_pointer;
    typedef typename node_allocator_t::nv_ref_t nv_reference;
    typedef typename traits_t::allocator_t allocator_t;

    // used only when locking allocator is present, otherwise resolves
    // to noops
    typename allocator_t::lock_counter lock_counter;

    node_allocator_t alloc;

public:
    InputIterator(node_handle_t node, const node_allocator_t& alloc) :
        current(node),
        alloc(alloc)
    {}

    //~InputIterator() {}

    static nv_reference lock(node_allocator_t& alloc, node_handle_t& handle_to_lock)
    {
        node_pointer p = alloc.lock(handle_to_lock);
        return traits_t::template value_exp<value_type>(*p);
    }

    // non standard handle-based mem helpers
    nv_reference lock() { return lock(alloc, current); }

    void unlock()
    {
        alloc.unlock(current);
    }


    // FIX: doing for(auto i : list) seems to do a *copy* operation
    // for(value_type& i : list) is required to get a reference.  Check to see if this is
    // proper behavior
    nv_reference operator*()
    {
        lock_counter++;
        // FIX: strong implications for leaving this unlocked,
        //      but in practice stronger implications for unlocking it.
        //      needs attention
        return lock();
    }

    bool operator==(const_iterator compare_to) const
    {
        return current == compare_to.current;
    }

    bool operator!=(const_iterator compare_to) const
    {
        return current != compare_to.current;
    }

    // FIX: call is ok but this should be protected/private and only accessible
    // via friends list/forward_list
    node_handle_t node() const { return current; }
};

template <class TValue, class TNodeTraits>
struct ForwardIterator : public InputIterator<TValue, TNodeTraits>
{
    typedef TNodeTraits traits_t;
    typedef InputIterator<TValue, TNodeTraits> base_t;
    typedef typename base_t::node_type   node_type;
    typedef typename base_t::value_type  value_type;
    typedef typename base_t::node_pointer node_pointer;
    typedef typename base_t::node_handle_t node_handle_t;
    typedef typename base_t::node_allocator_t node_alloc_t;
    typedef ForwardIterator<TValue, TNodeTraits> iterator;

    /*
    ForwardIterator(const ForwardIterator& source) :
            base_t(source)
    {
    } */

    ForwardIterator(node_handle_t node, const node_alloc_t& alloc) :
            base_t(node, alloc)
    {
    }


    ForwardIterator& operator++()
    {
        // special iterator behavior: unlocks any locks it itself
        // put into place.  Useful for iteration evaluation
        // operations using the lock() and unlock()
        // do not apply increment lock_counter - track locking for those with external means
        while(this->lock_counter > 0)
        {
            this->lock_counter--;
            this->unlock();
        }

        node_pointer c = base_t::alloc.lock(this->current);

        this->current = traits_t::get_next(*c);

        base_t::alloc.unlock(this->current);

        return *this;
    }

    // postfix version
    ForwardIterator operator++(int)
    {
        ForwardIterator temp(*this);
        operator++();
        return temp;
    }
};





template<class T, class TNodeTraits = node_traits_noalloc<T>>
class forward_list
{
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef ForwardIterator<value_type, node_traits_t> iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::allocator_t allocator_t;
    typedef typename node_traits_t::template test_node_allocator_t<value_type> node_allocator_t;
    typedef typename node_allocator_t::node_type node_type;
    typedef typename node_allocator_t::node_pointer node_pointer;

    typedef typename node_allocator_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

protected:
    node_allocator_t alloc;

    typedef typename node_traits_t::node_handle node_handle;

    static CONSTEXPR node_handle after_end_node() { return node_traits_t::null_node(); }

    node_handle m_front;

    node_handle next(node_handle from)
    {
        node_pointer f = alloc.lock(from);

        node_handle n = node_traits_t::get_next(*f);

        alloc.unlock(from);

        return n;
    }


    void set_next(node_handle _node, node_handle next)
    {
        node_pointer node = alloc.lock(_node);

        node_traits_t::set_next(*node, next);

        alloc.unlock(_node);
    }

public:
    forward_list(allocator_t* allocator = NULLPTR) :
        alloc(allocator),
        m_front(node_traits_t::null_node()) {}

    reference front()
    {
        reference front_value = iterator::lock(alloc, m_front);

        alloc.unlock(m_front);

        return front_value;
    }

    bool empty() const { return m_front == node_traits_t::null_node(); }

    void pop_front()
    {
#ifdef DEBUG
        // undefined behavior if list is empty, but let's put some asserts in here
#endif
        node_handle old = m_front;

        m_front = next(m_front);

        alloc.dealloc(old);
    }

    void push_front(nv_reference value)
    {
        node_handle node_pointing_to_value = alloc.alloc(value);

        if (m_front != node_traits_t::null_node())
        {
            set_next(node_pointing_to_value, m_front);
        }

        m_front = node_pointing_to_value;
    }

    iterator begin() { return iterator(m_front, alloc); }
    const_iterator begin() const { return iterator(m_front, alloc); }
    const_iterator end() const { return iterator(NULLPTR, alloc); }

    iterator insert_after(const_iterator pos, nv_reference value)
    {
        node_handle node_to_insert_after = pos.node();
        node_handle old_next_node = next(node_to_insert_after);
        node_handle node_pointing_to_value = alloc.alloc(value);

        set_next(node_to_insert_after, node_pointing_to_value);
        set_next(node_pointing_to_value, old_next_node);

        return iterator(node_pointing_to_value, alloc);
    }

    iterator erase_after(const_iterator pos)
    {
        node_handle node_to_erase_after = pos.node();
        node_handle node_to_erase = next(node_to_erase_after);
        node_handle node_following_erased = next(node_to_erase);

        set_next(node_to_erase_after, node_following_erased);

        return iterator(node_following_erased, alloc);
    }



    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    template <class UnaryPredicate>
    void remove_if(UnaryPredicate p, bool first_only = false)
    {
        node_handle current = m_front;
        node_handle previous;

        while(current != after_end_node())
        {
            node_pointer current_locked = alloc.lock(current);

            if(p(*current_locked))
            {
                set_next(previous, next(current));

                if(first_only) return;
            }

            alloc.unlock(current);

            previous = current;
            current = next(current);
        }
    }

    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    //NOTE: if C++03 will handle it, lean on remove_if instead of duplicating the code
    void remove(nv_reference value, bool first_only = false, bool pointer_comparison = false)
    {
        node_handle current = m_front;
        node_handle previous;

        while(current != after_end_node())
        {
            node_pointer current_locked = alloc.lock(current);

            // FIX: pointer comparison will do for now, but real
            // value-based == should eventually be used
            // FIX: also this won't work well for moveable/locked
            // memory chunks
            //if(current_locked == &value)
            bool match;

            if(pointer_comparison)
                match = current_locked == &value;
            else
                match = *current_locked == value;

            if(match)
            {
                set_next(previous, next(current));

                if(first_only) return;
            }

            alloc.unlock(current);

            previous = current;
            current = next(current);
        }


        //i.node()
    }
};

#ifdef UNUSED
// yanked directly in from util.embedded
// like it, but I don't like how node_traits directly reflect and require a list_type
namespace exp1 {


template <class TNode, class TAllocatorTraits = allocator_traits<Allocator<TNode>>>
class node_pointer
{
protected:
    typedef TNode node_type;
    typedef TAllocatorTraits node_allocator_traits;
    typedef node_pointer<node_type> this_t;
    typedef typename node_allocator_traits::pointer _node_pointer;

    _node_pointer current;

    node_pointer(node_type* current) : current(current) {}

public:
    _node_pointer getCurrent() const { return current; }

    bool operator==(const this_t& rhs)
    {
        return current == rhs.getCurrent();
    }

    bool operator!=(const this_t& rhs)
    {
        return current != rhs.getCurrent();
    }
};





template <class T, class TNode,
        class TNodeAllocatorTraits = allocator_traits<Allocator<TNode>>>
struct node_allocator
{
public:
    typedef T value_type;
    typedef TNode node_type;
    typedef TNodeAllocatorTraits node_allocator_traits;
    typedef allocator_traits<Allocator<value_type>> value_allocator_traits;

    typedef typename node_allocator_traits::pointer node_pointer;
    typedef typename value_allocator_traits::pointer value_pointer;

    typedef typename value_allocator_traits::const_void_pointer const_void_pointer;

    node_pointer allocate(value_pointer reference)
    {
        return reference;
    }

    void deallocate(node_pointer node)
    {
        // Maybe setting node->next to null here would be prudent?
    }

    // hint helps us track down what the associated value is
    // perhaps the node * is not in our control
    static value_pointer get_associated_value(node_pointer node, const_void_pointer hint)
    {
        return static_cast<value_pointer>(node);
    }
};


struct OutputIterator {};


template <class TNodeAllocator, const void* hint = nullptr,
        class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct InputIterator : public TBase
{
    typedef typename TNodeAllocator::value_type value_type;
    typedef typename TNodeAllocator::node_type node_type;
    typedef TBase base_t;

    InputIterator(node_type* node) : base_t(node) {}


    // FIX: doing for(auto i : list) seems to do a *copy* operation
    // for(value_type& i : list) is required to get a reference.  Check to see if this is
    // proper behavior
    value_type& operator*()
    {
        return *TNodeAllocator::get_associated_value(base_t::getCurrent(), hint);
    }
};



template <class TNodeAllocator, class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct ForwardIterator : public InputIterator<TNodeAllocator, nullptr, TBase>
{
    typedef typename TNodeAllocator::node_type   node_type;
    typedef typename TNodeAllocator::value_type  value_type;
    typedef InputIterator<TNodeAllocator, nullptr, TBase> base_t;

    ForwardIterator(const ForwardIterator& source) :
            base_t(source)
    {
    }

    ForwardIterator(node_type* node) :
            base_t(node)
    {
    }


    ForwardIterator& operator++()
    {
        base_t::current = node_traits<node_type>::get_next(base_t::current);
        return *this;
    }

    // postfix version
    ForwardIterator operator++(int)
    {
        ForwardIterator temp(*this);
        operator++();
        return temp;
    }
};


template <class TNodeAllocator, class TBase = node_pointer<typename TNodeAllocator::node_type>>
struct BidirectionalIterator : public ForwardIterator<TNodeAllocator, TBase>
{
    typedef typename TNodeAllocator::node_type   node_type;
    typedef typename TNodeAllocator::value_type  value_type;
    typedef ForwardIterator<TNodeAllocator, TBase> base_t;

    BidirectionalIterator(const BidirectionalIterator& source) :
            base_t(source)
    {
    }

    BidirectionalIterator(node_type* node) :
            base_t(node)
    {
    }


    BidirectionalIterator& operator--()
    {
        base_t::current = node_traits<node_type>::get_prev(base_t::current);
        return *this;
    }

    // postfix version
    BidirectionalIterator operator--(int)
    {
        BidirectionalIterator temp(*this);
        operator--();
        return temp;
    }
};


// TODO: move these to type_traits.h
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };


template <class TNodeAllocator, class TIterator>
class list_base
{
public:
    typedef typename TNodeAllocator::node_type node_type;
    typedef node_traits<node_type> node_traits_t;
    typedef typename node_traits_t::list_type list_type;
    typedef typename TNodeAllocator::value_type value_type;
    typedef typename TNodeAllocator::node_pointer node_pointer;
    typedef typename TNodeAllocator::value_pointer value_pointer;
    typedef list_base<TNodeAllocator, TIterator> this_t;

    typedef TIterator        iterator;
    typedef const iterator   const_iterator;

    typedef value_type&     reference;

protected:
    list_type list;
    TNodeAllocator node_allocator;

    TNodeAllocator& get_node_allocator()
    {
        return node_allocator;
    }

    inline node_pointer get_head() const
    {
        return node_traits_t::get_head(&list);
    }

    node_pointer _pop_front()
    {
        node_pointer node = get_head();
        list.experimental_set_head(node_traits_t::get_next(node));
        return node;
    }

public:
    bool empty() const { return get_head() == nullptr; }

    iterator begin() const { return iterator(get_head()); }
    iterator end() const { return iterator(nullptr); }

    reference front() const { return *begin(); }

    // not a const like in standard because we expect to actually modify
    // the prev/next parts of value
    void push_front(value_type& value)
    {
        node_pointer node = get_node_allocator().allocate(&value);

        list.insertAtBeginning(node);
    }


    void pop_front()
    {
        node_pointer node = _pop_front();
        get_node_allocator().deallocate(node);
    }

    friend struct helper;

    template <class T>
    struct helper
    {
        template <class Q = T>
        static typename enable_if<Q::has_tail() == true>::type
        handle_tail(this_t* t)
        {
            node_traits_t::get_tail(&t->list);
        }


        template <class Q = T>
        static typename enable_if<Q::has_tail() == false>::type
        handle_tail(this_t* t)
        {
        }
    };



    // TODO: determine if this works properly for double-linked as well
    // FIX: need to add "tail" awareness probably with SFINAE
    iterator erase_after(const_iterator pos)
    {
        node_pointer pos_node = pos.getCurrent();
        node_pointer node_to_erase = node_traits_t::get_next(pos_node);

        node_traits_t::remove_next(pos_node);

        // FIX: if node_to_erase == tail, we need to bump tail down by one,
        // effectively a pop_back
        helper<node_traits_t>::handle_tail(this);

        get_node_allocator().deallocate(node_to_erase);
        return iterator(node_traits_t::get_next(pos_node));
    }
};

template <class T, class TNodeAllocator = node_allocator<T, experimental::forward_node_base>>
class forward_list :
        public list_base<TNodeAllocator, ForwardIterator<TNodeAllocator>>
{
    typedef T value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    typedef list_base<TNodeAllocator, ForwardIterator<TNodeAllocator>> base_t;
    typedef typename base_t::node_traits_t node_traits_t;

    typedef typename base_t::node_type node_type;
    typedef typename base_t::node_pointer node_pointer;

    TNodeAllocator &get_node_allocator()
    {
        return base_t::node_allocator;
    }

public:
    typedef ForwardIterator<TNodeAllocator> iterator;
    typedef const iterator const_iterator;


    // non-standard:
    // removes/deallocate node at pos and splices in value
    const_iterator replace_after(const_iterator pos, value_type& value)
    {
        auto node_allocator = get_node_allocator();

        node_type* pos_node = pos.getCurrent();
        // old 'next' node prep it for erase
        node_type* node_to_erase = node_traits_t::get_next(pos_node);
        // new 'next' node allocate node portion, if necessary
        node_type* new_node = node_allocator.allocate(&value);

        // TODO: set pos_node->next to be &value
        // inset new 'next' node after current node and before old 'next''s next
        // node
        new_node->insertBetween(pos_node, node_to_erase->getNext());

        //
        node_allocator.deallocate(node_to_erase);

        return pos;
    }


    // Non-standard
    void replace_front(value_type& value)
    {
        auto node_allocator = get_node_allocator();

        node_type* front_node = base_t::get_head();

        node_type* new_front_node = node_allocator.allocate(value);

        new_front_node->insertBetween(nullptr, front_node->getNext());

        base_t::list.experimental_set_head(new_front_node);

    }


    iterator insert_after(const_iterator pos, value_type& value)
    {
        node_type* pos_node = pos.getCurrent();
        node_type* node = get_node_allocator().allocate(&value);

        // FIX: insertBetween is overcompliated, the insert_after is cleaner and better
        // (the getNext() is always the value used, so why bother making it an explicit param)
        base_t::list.insertBetween(pos_node, pos_node->getNext(), node);

        return iterator(node);
    }

    // Non-standard, eliminate this call in favor of more manual pop_front/etc
    void remove(reference r)
    {
        base_t::list.remove(&r);
    }

};



template <class T, class TNodeAllocator = node_allocator<T, experimental::double_node_base>>
class list :
        public list_base<TNodeAllocator, BidirectionalIterator<TNodeAllocator>>
{
    typedef T                   value_type;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef list_base<TNodeAllocator, BidirectionalIterator<TNodeAllocator>> base_t;
    typedef typename base_t::node_traits_t  node_traits_t;

    typedef typename base_t::node_type      node_type;
    typedef typename base_t::node_pointer   node_pointer;

    TNodeAllocator& get_node_allocator()
    {
        return base_t::node_allocator;
    }

public:
    typedef BidirectionalIterator<TNodeAllocator>         iterator;
    typedef const iterator   const_iterator;

    reference back()
    {
        node_pointer ptr = base_t::list.experimental_get_tail();
        reference value = get_node_allocator().get_associated_value(ptr);
        return value;
    }

    void pop_back()
    {
        // If not empty
        if(!base_t::empty())
        {
            // grab very last entry
            node_pointer tail = base_t::list.experimental_get_tail();

            // then grab entry just before last entry
            node_pointer prev = node_traits_t::get_prev(tail);

            // FIX: removeNext does more work than we need, it
            // actually does prev->next = prev->next->next
            prev->removeNext();

            // if necessary, deallocate memory for tail node
            get_node_allocator().deallocate(tail);

            // now set tail to previous entry
            base_t::list.experimental_set_tail(prev);
        }
    }
};
}
#endif
}
