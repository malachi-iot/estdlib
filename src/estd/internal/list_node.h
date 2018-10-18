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

    void set_next(node_handle _node, node_handle next)
    {
        node_type& node = alloc_lock(_node);

        get_traits().next(node, next);

        alloc_unlock(_node);
    }
};


// kind of implicitly the 'linkedlist_front' + linked list forward
// rolled together
template<class TNodeTraits, class TIterator>
class linkedlist_base : public linkedlist_traits_base<TNodeTraits, TIterator>
{
    typedef linkedlist_traits_base<TNodeTraits, TIterator> base_type;

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

    node_traits_t& get_traits() { return base_type::get_traits(); }

    const node_traits_t& get_traits() const { return base_type::get_traits(); }

protected:

    static CONSTEXPR node_handle after_end_node() { return node_traits_t::eol(); }

    node_handle m_front;

    linkedlist_base(allocator_t* a) :
            m_front(after_end_node())

    {}

    linkedlist_base(node_traits_t& traits) :
        base_type(traits),
        m_front(after_end_node())
    {

    }

    void set_front(node_handle new_front)
    {
        base_type::set_next(new_front, m_front);

        m_front = new_front;
    }


public:
    bool empty() const { return m_front == after_end_node(); }

    // TODO: move towards 'accessor' pattern instead of direct references, not as
    // critical as with dynamic_array however, since these references are not
    // expected directly to move (the underlying node itself may, however)
    reference front()
    {
        //reference front_value = iterator::lock(base_t::_alloc, base_t::m_front);
        reference front_value = base_type::alloc_lock(m_front);

        base_type::alloc_unlock(m_front);

        return front_value;
    }

    iterator begin() { return iterator(m_front, get_traits()); }
    const_iterator begin() const
    {
        const node_traits_t& t = get_traits();
        return iterator(m_front, t);
    }
    const_iterator end() const { return iterator(after_end_node(), get_traits()); }

#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
    iterator before_begin() { return iterator(m_front, get_traits(), true); }
#endif
};

template<class TNodeTraits, class TIterator>
class linkedlist_back_base : public linkedlist_base_base<TNodeTraits, TIterator>
{
    typedef linkedlist_base_base<TNodeTraits, TIterator> base_type;

    typedef typename base_type::node_handle node_handle;
    typedef typename base_type::reference reference;

protected:
    node_handle m_back;

public:
    reference back()
    {
        //reference front_value = iterator::lock(base_t::_alloc, base_t::m_front);
        reference back_value = base_type::alloc_lock(m_back);

        base_type::alloc_unlock(m_back);

        return back_value;
    }
};

}

}
