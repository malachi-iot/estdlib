#pragma once

#include "../internal/platform.h"
#include "../type_traits.h"

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

namespace internal
{
template<class TNodeTraits, class TIterator>
class linkedlist_base_base
{
public:
    typedef typename estd::remove_reference<TNodeTraits>::type node_traits_t;
    typedef typename node_traits_t::value_type value_type;
    typedef value_type& reference;
    typedef TIterator iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::node_allocator_type node_allocator_t;
    typedef node_allocator_t allocator_t;
    typedef typename node_traits_t::node_type node_type;
    //typedef typename node_allocator_t::node_pointer node_pointer;

    typedef typename node_traits_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

    typedef allocator_traits<allocator_t> allocator_traits_t;

    typedef typename node_traits_t::node_handle node_handle;


};

template<class TNodeTraits, class TIterator>
class linkedlist_traits_base : public linkedlist_base_base<TNodeTraits, TIterator>
{
    typedef linkedlist_base_base<TNodeTraits, TIterator> base_type;

public:
    typedef typename base_type::node_traits_t node_traits_t;
    typedef typename base_type::node_type node_type;
    typedef typename base_type::node_handle node_handle;

protected:
    // for now assume stateful traits (contains allocators)
    // optimize this out later
    TNodeTraits traits;

    linkedlist_traits_base() {}

    linkedlist_traits_base(node_traits_t& traits) :
    traits(traits)
            {

            }

    node_traits_t& get_traits() { return traits; }

    const node_traits_t& get_traits() const { return traits; }

    node_type& alloc_lock(node_handle& to_lock)
    {
        return get_traits().lock(to_lock);
    }

    void alloc_unlock(node_handle& to_unlock)
    {
        get_traits().unlock(to_unlock);
    }


    node_handle next(node_handle from)
    {
        node_type& f = alloc_lock(from);

        node_handle n = get_traits().next(f);

        alloc_unlock(from);

        return n;
    }

    node_handle prev(node_handle from)
    {
        node_type& f = alloc_lock(from);

        node_handle n = get_traits().prev(f);

        alloc_unlock(from);

        return n;
    }

    // presumes forward-capable list - may not actually be the case, but usually is.
    // If not, probably template semi-late binding will keep this from compiler erroring
    void set_next(node_handle _node, node_handle next)
    {
        node_type& node = alloc_lock(_node);

        get_traits().next(node, next);

        alloc_unlock(_node);
    }
};

template<class TNodeTraits, class TIterator>
class linkedlist_front_base : public linkedlist_traits_base<TNodeTraits, TIterator>
{
    typedef linkedlist_traits_base<TNodeTraits, TIterator> base_type;
public:
    typedef typename base_type::node_traits_t node_traits_t;
    typedef typename node_traits_t::value_type value_type;
    typedef value_type& reference;
    typedef typename node_traits_t::node_type node_type;
    typedef typename node_traits_t::node_handle node_handle;
    typedef typename node_traits_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

protected:
    node_handle m_front;

    linkedlist_front_base(node_handle f) : m_front(f) {}

    linkedlist_front_base(node_traits_t& traits, node_handle f) :
        base_type(traits),
        m_front(f) {}

public:
    // TODO: move towards 'accessor' pattern instead of direct references, not as
    // critical as with dynamic_array however, since these references are not
    // expected directly to move (the underlying node itself may, however)
    reference front()
    {
        // NOTE: beware, the condition could arise where:
        // a) the underlying allocator really is locking and
        // b) it's an inline node-value pair
        // in which case, when both are true, front_value will actually be invalid
        // so the 'accessor' pattern is of increased importance to stave that off
        reference front_value = base_type::alloc_lock(m_front);

        base_type::alloc_unlock(m_front);

        return front_value;
    }

};

// kind of implicitly the 'linkedlist_front' + linked list forward
// rolled together
template<class TNodeTraits, class TIterator>
class forward_list_base : public linkedlist_front_base<TNodeTraits, TIterator>
{
    typedef linkedlist_front_base<TNodeTraits, TIterator> base_type;

public:
    typedef typename base_type::node_traits_t node_traits_t;
    typedef typename node_traits_t::value_type value_type;
    typedef value_type& reference;
    typedef TIterator iterator;
    typedef const iterator   const_iterator;
    typedef typename node_traits_t::node_allocator_type node_allocator_t;
    typedef node_allocator_t allocator_t;
    typedef typename node_traits_t::node_type node_type;
    //typedef typename node_allocator_t::node_pointer node_pointer;

    typedef typename node_traits_t::nv_ref_t nv_ref_t;
    typedef nv_ref_t nv_reference;

    typedef allocator_traits<allocator_t> allocator_traits_t;

    typedef typename node_traits_t::node_handle node_handle;

    node_traits_t& get_traits() { return base_type::get_traits(); }

    const node_traits_t& get_traits() const { return base_type::get_traits(); }

protected:

    static CONSTEXPR node_handle after_end_node() { return node_traits_t::eol(); }

    // FIX: Definitely a bad thing to have this dummy allocator pass in this particular way
    // - not expressly bad to be passing in an allocator in general though
    forward_list_base(allocator_t* a) :
            base_type(after_end_node())

    {}

    forward_list_base(node_traits_t& traits) :
        base_type(traits, after_end_node())
    {

    }

    void set_front(node_handle new_front)
    {
        base_type::set_next(new_front, base_type::m_front);

        base_type::m_front = new_front;
    }


public:
    bool empty() const { return base_type::m_front == after_end_node(); }

    iterator begin() { return iterator(base_type::m_front, get_traits()); }
    const_iterator begin() const
    {
        const node_traits_t& t = get_traits();
        return iterator(base_type::m_front, t);
    }
    const_iterator end() const { return iterator(after_end_node(), get_traits()); }

#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
    iterator before_begin() { return iterator(m_front, get_traits(), true); }
#endif
};

template<class TNodeTraits, class TIterator,
        class TBase = linkedlist_base_base<TNodeTraits, TIterator> >
class linkedlist_back_base : public TBase
{
    typedef TBase base_type;

public:
    typedef typename base_type::node_handle node_handle;
    typedef typename base_type::reference reference;

protected:
    node_handle m_back;

    // Initial value - should always be the NULL/eol value
    linkedlist_back_base(node_handle b) : m_back(b) {}

public:
    // all the same caveats to front() apply here
    reference back()
    {
        reference back_value = base_type::alloc_lock(m_back);

        base_type::alloc_unlock(m_back);

        return back_value;
    }
};

template<class TNodeTraits, class TIterator, class TBase = forward_list_base<TNodeTraits, TIterator> >
class forward_list_with_back_base : public
        linkedlist_back_base<TNodeTraits, TIterator, TBase >
{
    typedef linkedlist_back_base<TNodeTraits, TIterator, TBase > base_type;

public:
    typedef typename base_type::node_handle node_handle;
    typedef typename base_type::nv_reference nv_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;

private:
    // disable these calls, as they do not yet account for 'back' and can get things out of sync
    // fix them with impl version.  holding off until then - anticipate impl version will also
    // help solve the stateful-traits awkwardness
    iterator insert_after(const_iterator pos, nv_reference value);
    iterator erase_after(const_iterator pos);
#ifdef FEATURE_CPP_VARIADIC
    template<class... TArgs>
    node_handle emplace_front(TArgs&& ... args);
#endif
    template<class UnaryPredicate>
    void remove_if(UnaryPredicate p, bool first_only = false);
    void remove(nv_reference value, bool first_only = false);

protected:
    // this particular set_back is going to handle m_front, too, if necessary
    // TODO: move the set_back and set_front to an impl, so as to more easily share them
    // accross strata of list handlers
    void set_back(node_handle new_back)
    {
        // are we tracking anything?
        if(base_type::m_back != base_type::after_end_node())
        {
            // set the current 'back' next-position to link to the new back
            // TODO: could assert that current m_back->next is NULL
            base_type::set_next(base_type::m_back, new_back);
        }
        else
        {
            // give attention to m_front here too, since m_back == NULL means m_front == NULL
            base_type::m_front = new_back;
        }

        // set the current 'back' tracker to this new_back
        base_type::m_back = new_back;
    }


    void set_front(node_handle new_front)
    {
        base_type::set_next(new_front, base_type::m_front);

        base_type::m_front = new_front;

        // if no back tracked at all, time to start
        // otherwise, it's assumed old m_front is already pointing its next (eventually)
        // at existing m_back, so we should be good to go
        if(base_type::m_back == base_type::after_end_node())
        {
            base_type::m_back = new_front;
        }
    }


public:
    forward_list_with_back_base() :
        base_type(base_type::after_end_node())
    {}

    void push_back(nv_reference value)
    {
        node_handle h = base_type::get_traits().allocate(value);
        set_back(h);
    }

    void push_front(nv_reference value)
    {
        set_front(base_type::get_traits().allocate(value));
    }

};

}
}
