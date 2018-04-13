#pragma once

#include "../platform.h"
#include "memory.h"

namespace estd {

template <class TNode, class TAllocator = void> struct node_traits;

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
    T &value() { return m_value; }

    const forward_node<T> *next() const {
        return (forward_node<T> *) forward_node_base::next();
    }

    void next(forward_node<T> *set_to) {
        forward_node_base::next(set_to);
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

// this is where node and value are combined, and no allocator is used
// (node memory management entirely external to node and list)
template<class TForwardNodeValue, class TAllocator>
struct node_traits
{
    typedef TForwardNodeValue value_type;
    typedef value_type node_type;
    // represents EITHER const value ref or non-const node+value ref
    typedef value_type& nv_reference;

    // TODO: eventually interact with allocator for this (in
    // other node_traits where allocation actually happens)
    typedef node_type* node_pointer;
    typedef node_type* node_handle;
    typedef TAllocator allocator_t;

    static node_handle null_node() { return NULLPTR; }

    static node_handle get_next(const node_type& node)
    {
        // NOTE: we assume that node_type represents a very specific type derived ultimately
        // from something resembling forward_node_base, specifically in that
        // a call to next() shall return a pointer to the next node_type*
        return reinterpret_cast<node_type*>(node.next());
    }

    static node_handle get_prev(const node_type& node)
    {
        return reinterpret_cast<node_type*>(node.prev());
    }

    static void set_next(node_type& node, node_handle set_to)
    {
        node.next(set_to);
    }

    // replacement for old allocator get associated value
    static value_type& value(node_type& node) { return node; }

    // pretends to allocate space for a node, when in fact no allocation
    // is necessary for this type
    static node_handle alloc_node(allocator_t* a, value_type& value) { return &value; }

    static void dealloc_node(allocator_t* a, node_handle node) {}

    // placeholders
    // only useful when a) list is managing node memory allocations and
    // b) when they are handle-based
    static node_pointer lock(node_handle node) { return node; }
    static void unlock(node_handle node) {}

    node_traits(allocator_t* a) {}
};


template<>
struct node_traits<experimental::forward_node_base>
{
    typedef experimental::forward_node_base node_type;

    static const node_type *get_next(const node_type &node) { return node.next(); }

    static void set_next(node_type &node, node_type *set_to) { node.next(set_to); }
};


// adapted from util.embedded version
template <class TNodeTraits>
struct InputIterator
{
    typedef typename TNodeTraits::value_type value_type;
    typedef typename TNodeTraits::node_type node_type;
    typedef typename TNodeTraits::node_handle node_handle_t;

protected:
    node_handle_t current;

public:
    InputIterator(node_handle_t node) : current(node) {}


    // FIX: doing for(auto i : list) seems to do a *copy* operation
    // for(value_type& i : list) is required to get a reference.  Check to see if this is
    // proper behavior
    value_type& operator*()
    {
        //return *TNodeAllocator::get_associated_value(base_t::getCurrent(), hint);
        return TNodeTraits::value(*current);
    }

    bool operator==(const InputIterator<TNodeTraits>& compare_to) const
    {
        return current == compare_to.current;
    }

    bool operator!=(const InputIterator<TNodeTraits>& compare_to) const
    {
        return current != compare_to.current;
    }

    // FIX: call is ok but this should be protected/private and only accessible
    // via friends list/forward_list
    node_handle_t node() const { return current; }
};

template <class TNodeTraits>
struct ForwardIterator : public InputIterator<TNodeTraits>
{
    typedef TNodeTraits traits_t;
    typedef InputIterator<TNodeTraits> base_t;
    typedef typename base_t::node_type   node_type;
    typedef typename base_t::value_type  value_type;
    typedef typename base_t::node_handle_t node_handle_t;

    /*
    ForwardIterator(const ForwardIterator& source) :
            base_t(source)
    {
    } */

    ForwardIterator(node_handle_t node) :
            base_t(node)
    {
    }


    ForwardIterator& operator++()
    {
        node_handle_t c = traits_t::lock(this->current);

        this->current = traits_t::get_next(*c);

        traits_t::unlock(this->current);

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





template<class T, class TNodeTraits = node_traits<T>>
class forward_list
{
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef typename node_traits_t::node_type node_type;
    typedef typename node_traits_t::nv_reference nv_reference;
    typedef ForwardIterator<node_traits_t> iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::allocator_t allocator_t;

protected:
    node_traits_t node_traits;

    typedef typename node_traits_t::node_pointer node_pointer;
    typedef typename node_traits_t::node_handle node_handle;

    node_handle m_front;

    // FIX: Consider *not* using this as it adds unecessary overhead to scenarios
    // which don't do locking (oops)
    // -OR- put lock_helper itself into allocator_traits or similar so that we only
    // incur its overhead of locking is actually going on
    // eventually that may be a kind of unique_ptr-like scenario
    struct lock_helper
    {
        node_handle h;
        node_pointer ptr;

        lock_helper(node_handle h) : h(h)
        {
            ptr = node_traits_t::lock(h);
        }

        ~lock_helper()
        {
            node_traits_t::unlock(h);
        }

        node_pointer& operator*() { return ptr; }
    };

    static node_pointer next(node_type& from)
    {
        return node_traits_t::get_next(from);
    }


    static node_handle next(node_handle from)
    {
        lock_helper f(from);

        return node_traits_t::get_next(*f.ptr);
    }

    /*
    static void set_next(node_type& node, node_pointer next)
    {
        node_traits_t::set_next(node, next);
    }*/

    static void set_next(node_type& node, node_handle next)
    {
        node_traits_t::set_next(node, next);
    }

public:
    forward_list(allocator_t* allocator = NULLPTR) :
        node_traits(allocator),
        m_front(node_traits_t::null_node()) {}

    reference front()
    {
        node_pointer p = node_traits_t::lock(m_front);
        value_type& front_value = node_traits_t::value(*p);
        node_traits_t::unlock(m_front);

        return front_value;
    }

    bool empty() const { return m_front == NULLPTR; }

    void pop_front()
    {
#ifdef DEBUG
        // undefined behavior if list is empty, but let's put some asserts in here
#endif
        m_front = next(m_front);
    }

    void push_front(nv_reference value)
    {
        node_handle node_pointing_to_value = node_traits_t::alloc_node(NULLPTR, value);

        if (m_front != node_traits_t::null_node())
        {
            node_pointer n = node_traits_t::lock(node_pointing_to_value);
            set_next(*n, m_front);
            node_traits_t::unlock(node_pointing_to_value);
        }

        m_front = node_pointing_to_value;
    }

    iterator begin() { return iterator(m_front); }
    const_iterator begin() const { return iterator(m_front); }
    const_iterator end() const { return iterator(NULLPTR); }

    iterator insert_after(const_iterator pos, nv_reference value)
    {
        node_pointer node_to_insert_after = pos.node();
        node_pointer old_next_node = next(*node_to_insert_after);
        node_handle node_pointing_to_value = node_traits_t::alloc_node(NULLPTR, value);

        set_next(*node_to_insert_after, node_pointing_to_value);
        set_next(*node_pointing_to_value, old_next_node);

        return iterator(node_pointing_to_value);
    }
};

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

}
