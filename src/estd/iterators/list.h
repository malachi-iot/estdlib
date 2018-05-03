#pragma once

namespace estd {

// adapted from util.embedded version
template <class TValue, class TNodeTraits>
struct InputIterator
{
    typedef TNodeTraits traits_t;
    typedef TValue value_type;
    //typedef typename traits_t::template node_allocator_t<value_type> node_allocator_t;
    typedef typename traits_t::node_allocator_type node_allocator_t;
    //typedef typename TNodeTraits::node_handle node_handle_t;
    typedef InputIterator<TValue, TNodeTraits> iterator;
    typedef const iterator const_iterator;

protected:
    //typedef typename node_allocator_t::node_handle node_handle_t;
    typedef typename traits_t::node_handle node_handle_t;

    node_handle_t current;


    //typedef typename traits_t::node_allocator_t node_alloc_t;
    typedef typename traits_t::node_type node_type;
    typedef node_type* node_pointer;
    typedef typename traits_t::nv_ref_t nv_reference;
    typedef node_allocator_t allocator_t;

    typedef allocator_traits<allocator_t> allocator_traits_t;

    // used only when locking allocator is present, otherwise resolves
    // to noops
    typename allocator_t::lock_counter lock_counter;

    traits_t traits;

    node_type& lock_internal()
    {
        return traits.lock(current);
    }

    void unlock_internal()
    {
        traits.unlock(current);
    }

public:
    InputIterator(node_handle_t node, const traits_t& traits) :
        current(node),
        traits(traits)
    {}

    //~InputIterator() {}

    static nv_reference lock(traits_t& a, node_handle_t& handle_to_lock)
    {
        node_type& p = a.lock(handle_to_lock);
        return traits_t::value(p);
    }

    // non standard handle-based mem helpers
    nv_reference lock() { return lock(traits, current); }

    void unlock()
    {
        traits.unlock(current);
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

    ForwardIterator(node_handle_t node, const traits_t& t) :
            base_t(node, t)
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

        node_type& c = base_t::lock_internal();

        this->current = base_t::traits.next(c);

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
