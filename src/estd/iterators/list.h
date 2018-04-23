#pragma once

namespace estd {

// adapted from util.embedded version
template <class TValue, class TNodeTraits>
struct InputIterator
{
    typedef TNodeTraits traits_t;
    typedef TValue value_type;
    typedef typename traits_t::template node_allocator_t<value_type> node_allocator_t;
    //typedef typename TNodeTraits::node_handle node_handle_t;
    typedef InputIterator<TValue, TNodeTraits> iterator;
    typedef const iterator const_iterator;

protected:
    typedef typename node_allocator_t::node_handle node_handle_t;

    node_handle_t current;


    //typedef typename traits_t::node_allocator_t node_alloc_t;
    typedef typename node_allocator_t::node_type node_type;
    typedef typename node_allocator_t::node_pointer node_pointer;
    typedef typename node_allocator_t::nv_ref_t nv_reference;
    typedef typename node_allocator_t::allocator_t allocator_t;

    typedef allocator_traits<allocator_t> allocator_traits_t;

    // used only when locking allocator is present, otherwise resolves
    // to noops
    typename allocator_t::lock_counter lock_counter;

    node_allocator_t alloc;
    // temporary until we decouple from node_allocator_t
    allocator_t _alloc;

    node_type& lock_internal()
    {
        return allocator_traits_t::lock(_alloc, current);
    }

    void unlock_internal()
    {
        allocator_traits_t::unlock(_alloc, current);
    }

public:
    InputIterator(node_handle_t node, const node_allocator_t& alloc) :
        current(node),
        alloc(alloc)
        //_alloc(_alloc)
    {}

    //~InputIterator() {}

    static nv_reference lock(allocator_t& a, node_handle_t& handle_to_lock)
    {
        node_type& p = allocator_traits_t::lock(a, handle_to_lock);
        return traits_t::template value_exp<value_type>(p);
    }

    // non standard handle-based mem helpers
    nv_reference lock() { return lock(_alloc, current); }

    void unlock()
    {
        allocator_traits_t::unlock(_alloc, current);
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

        //node_type& c = base_t::alloc.lock(this->current);
        node_type& c = base_t::lock_internal();

        // FIX: static cast from node_handle_base to node_handle
        this->current = (node_handle_t) traits_t::get_next(c);

        //base_t::alloc.unlock(this->current);
        base_t::unlock_internal();

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


}
