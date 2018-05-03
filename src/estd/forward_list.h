#pragma once

#include "../platform.h"
#include "memory.h"
#include "traits/node_list.h"
#include "iterators/list.h"

#ifdef FEATURE_CPP_MOVESEMANTIC
#include <utility>
#endif

namespace estd {

namespace experimental {

// Make list and forward list conform to, but not necessarily depend on, these
// signatures (C++ concepts would be nice here)
// TODO: fixup these names and move them out of experimental.  Remember, although
// all our node_traits and friends expect these signatures, you could roll your
// own node traits and use different signatures along the way

// TNode might be a handle instead of a direct pointer.  Perhaps our own kind of
// "fancy pointer"
template <class TNode>
class forward_node_base_base
{
protected:

    TNode m_next;

    forward_node_base_base(TNode initial_value) : m_next(initial_value) {}

public:
    typedef TNode node_t;

    node_t next() const { return m_next; }

    void next(node_t set_to) { m_next = set_to; }
};

class forward_node_base : public forward_node_base_base<forward_node_base*>
{
protected:
    typedef forward_node_base_base<forward_node_base*> base_t;

public:
    forward_node_base() : base_t(NULLPTR) {}
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

    forward_node<T> *next() const {
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
template <class TNode, class TValue, class TAllocator>
struct node_value_traits_experimental<
        typename smart_node_alloc<TNode, TAllocator>::template RefNode<TValue>, TValue>
{

    //const TValue& value()
};
*/

namespace internal
{
template<class T, class TNodeTraits>
class linkedlist_base
{
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef ForwardIterator<value_type, node_traits_t> iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::template node_allocator_t<value_type> node_allocator_t;
    typedef typename node_allocator_t::allocator_t allocator_t;
    typedef typename node_allocator_t::node_type node_type;
    typedef typename node_allocator_t::node_pointer node_pointer;

    typedef typename node_allocator_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

    typedef allocator_traits<allocator_t> allocator_traits_t;

protected:
    typedef typename node_allocator_t::node_handle node_handle;

    static CONSTEXPR node_handle after_end_node() { return (node_handle) node_traits_t::null_node(); }

    node_allocator_t alloc;
    allocator_t _alloc; // Temporary only until we divest alloc operations completely from node allocator

    node_handle m_front;

    linkedlist_base(allocator_t* a) :
            alloc(a),
    //_alloc(allocator),
            m_front(after_end_node())

    {}
};

}

// TODO: Due to the requisite deviation for Allocator to take a <T>,
// consider adding Allocator (<T>) to forward list and yanking all allocator
// direct references out of TNodeTraits.  TNodeTraits will have to rely on
// incoming templatized allocator calls.  One possibility is TNodeTrait can
// present a 'default' allocator , though this might conflict with a default of
// std::allocator<T> specialization, but that itself is deprecated for C++17
template<class T, class TNode = T,
         class TAllocator = experimental_std_allocator<TNode>,
         class TNodeTraits = node_traits<T, TAllocator > >
class forward_list : public internal::linkedlist_base<T, TNodeTraits>
{
public:
    typedef T value_type;
    typedef value_type& reference;
    typedef TNodeTraits node_traits_t;
    typedef ForwardIterator<value_type, node_traits_t> iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::template node_allocator_t<value_type> node_allocator_t;
    typedef typename node_allocator_t::allocator_t allocator_t;
    typedef typename node_allocator_t::node_type node_type;
    typedef typename node_allocator_t::node_pointer node_pointer;

    typedef typename node_allocator_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

    typedef allocator_traits<allocator_t> allocator_traits_t;

protected:
    typedef internal::linkedlist_base<T, TNodeTraits> base_t;

    typedef typename node_allocator_t::node_handle node_handle;
    //typedef typename node_traits_t::node_handle node_handle;

    // FIX: kill these forward casts
    static CONSTEXPR node_handle after_end_node() { return (node_handle) node_traits_t::null_node(); }
    static CONSTEXPR node_handle before_beginning_node() { return (node_handle) node_traits_t::null_node(); }

    node_type& alloc_lock(node_handle& to_lock)
    {
        return allocator_traits_t::lock(base_t::_alloc, to_lock);
    }

    void alloc_unlock(node_handle& to_unlock)
    {
        allocator_traits_t::unlock(base_t::_alloc, to_unlock);
    }

    node_handle next(node_handle from)
    {
        node_type& f = alloc_lock(from);

        // FIX: have to forward cast from node_handle_base to node_handle
        node_handle n = (node_handle) node_traits_t::get_next(f);

        alloc_unlock(from);

        return n;
    }


    void set_next(node_handle _node, node_handle next)
    {
        node_type& node = alloc_lock(_node);

        node_traits_t::set_next(node, next);

        alloc_unlock(_node);
    }


    void set_front(node_handle new_front)
    {
        set_next(new_front, base_t::m_front);

        base_t::m_front = new_front;
    }

public:
    forward_list(allocator_t* allocator = NULLPTR) :
        base_t(allocator) {}

    // TODO: move towards 'accessor' pattern instead of direct references, not as
    // critical as with dynamic_array however, since these references are not
    // expected directly to move (the underlying node itself may, however)
    reference front()
    {
        reference front_value = iterator::lock(base_t::_alloc, base_t::m_front);

        alloc_unlock(base_t::m_front);

        return front_value;
    }

    bool empty() const { return base_t::m_front == after_end_node(); }

    void pop_front()
    {
#ifdef DEBUG
        // undefined behavior if list is empty, but let's put some asserts in here
#endif
        node_handle old = base_t::m_front;

        base_t::m_front = next(base_t::m_front);

        base_t::alloc.dealloc(old);
    }

    void push_front(nv_reference value)
    {
        set_front(base_t::alloc.alloc(value));
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void push_front(value_type&& value)
    {
        set_front(base_t::alloc.alloc_move(std::forward<value_type>(value)));
    }
#endif


    iterator begin() { return iterator(base_t::m_front, base_t::alloc); }
    const_iterator begin() const { return iterator(base_t::m_front, base_t::alloc); }
    const_iterator end() const { return iterator(NULLPTR, base_t::alloc); }

    iterator insert_after(const_iterator pos, nv_reference value)
    {
        node_handle node_to_insert_after = pos.node();
        node_handle old_next_node = next(node_to_insert_after);
        node_handle node_pointing_to_value = base_t::alloc.alloc(value);

        set_next(node_to_insert_after, node_pointing_to_value);
        set_next(node_pointing_to_value, old_next_node);

        return iterator(node_pointing_to_value, base_t::alloc);
    }

    iterator erase_after(const_iterator pos)
    {
        node_handle node_to_erase_after = pos.node();
        node_handle node_to_erase = next(node_to_erase_after);
        node_handle node_following_erased = next(node_to_erase);

        set_next(node_to_erase_after, node_following_erased);

        return iterator(node_following_erased, base_t::alloc);
    }

#ifdef FEATURE_CPP_VARIADIC

    // a bit tricky because we have 4 allocation possibilities:
    // 'intrusive' where node data itself is a resident of value_type itself, externally allocated
    // 'non-intrusive' where node data is managed via a reference (RefNode) - this right now
    //                 only works with externally-allocated refs.  Ultimately this would split
    //                 into RefNode and RefNodeManaged where the latter allocs and deletes
    //                 its own value_type
    // 'semi-intrusive' where really it is intrusive but in reverse, forward_list allocates
    //                  both the node and value_type together as part of the node.  In effect
    //                  keeps copies of items placed into array
    template <class... TArgs>
    node_handle emplace_front( TArgs&&... args )
    {
        static_assert(node_allocator_t::can_emplace(), "This allocator cannot emplace");

        node_handle h = base_t::alloc.alloc_emplace(args...);

        set_front(h);

        // deviates from std::forward_list in that that returns
        // a reference, but we return a handle which will need
        // a lock()
        return h;
    }
#endif


    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    template <class UnaryPredicate>
    void remove_if(UnaryPredicate p, bool first_only = false)
    {
        node_handle current = base_t::m_front;
        node_handle previous = before_beginning_node();

        while(current != after_end_node())
        {
            node_handle _next = next(current);
            node_type& current_locked = alloc_lock(current);

            bool matched = p(current_locked);

            alloc_unlock(current);

            if(matched)
            {
                // If we match but there's no previous node
                if(previous == before_beginning_node())
                    // then instead of splicing, we are replacing the front node
                    base_t::m_front = _next;
                else
                    set_next(previous, _next);

                if(first_only) return;
            }

            previous = current;
            current = _next;
        }
    }

    // deviates from std C++ in that this optionally shall remove the first item found
    // in the list, rather than all items matching
    //NOTE: if C++03 will handle it, lean on remove_if instead of duplicating the code
    //TODO: Unlikely C++03 wil handle that, so instead do a #define to wrap up predicate
    // portion and fold into one remove_internal which is called differently depending
    // on C++03 or C++11
    void remove(nv_reference value, bool first_only = false)
    {
#ifdef FEATURE_CPP_LAMBDA
        remove_if([&value](nv_reference compare_to)
                  {
                      return value == compare_to;
                  }, first_only );
#else
        node_handle current = base_t::m_front;
        node_handle previous = before_beginning_node();

        while(current != after_end_node())
        {
            node_handle _next = next(current);
            node_type& current_locked = alloc_lock(current);

            bool matched = current_locked == value;

            alloc_unlock(current);

            if(matched)
            {
                // If we match but there's no previous node
                if(previous == node_traits_t::null_node())
                    // then instead of splicing, we are replacing the front node
                    base_t::m_front = _next;
                else
                    set_next(previous, next(current));

                if(first_only) return;
            }

            previous = current;
            current = _next;
        }
#endif
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
