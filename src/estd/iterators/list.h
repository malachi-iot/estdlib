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

#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    // used only when locking allocator is present, otherwise resolves
    // to noops
    typename allocator_t::lock_counter lock_counter;
#endif

    // FIX: Pretty sure we want this to be a reference, not a value
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
        node_type& n = a.lock(handle_to_lock);
        return traits_t::value(n);
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
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
        lock_counter++;
#endif
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


namespace internal { namespace list {

// TODO: Rename or put into namespace to isolate this specifically for NODE iteration
template <class TValue, class TNodeTraits, class TBase = InputIterator<TValue, TNodeTraits> >
struct ForwardIterator : public TBase
{
    // special mode for iterator representing before beginning.  we use NULL (invalid())
    // to designate end, so we need this to disambiguate
    // consider this experimental and unfortunately it fattens up the iterator as well
    bool before_beginning;

    typedef TNodeTraits traits_t;
    typedef TBase base_t;
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
            base_t(node, t),
            before_beginning(false)
    {
    }


    ForwardIterator& operator++()
    {
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
        // special iterator behavior: unlocks any locks it itself
        // put into place.  Useful for iteration evaluation
        // operations using the lock() and unlock()
        // do not apply increment lock_counter - track locking for those with external means
        while(this->lock_counter > 0)
        {
            this->lock_counter--;
            this->unlock();
        }
#endif
        if(before_beginning)
        {
            before_beginning = false;
            return *this;
        }

        node_type& c = base_t::lock_internal();

        node_handle_t new_current = base_t::traits.next(c);

        base_t::unlock_internal();

        this->current = new_current;

        return *this;
    }

    // postfix version
    ForwardIterator operator++(int)
    {
        ForwardIterator temp(*this);
        operator++();
        return temp;
    }


    ForwardIterator& operator +=(int summand)
    {
        while(summand--) operator ++();
        return *this;
    }


    ForwardIterator operator +(int summand) const
    {
        ForwardIterator temp(*this);

        return temp += summand;
    }

    bool operator==(const ForwardIterator& compare_to) const
    {
        return before_beginning == compare_to.before_beginning &&
                base_t::current == compare_to.current;
    }

    bool operator!=(const ForwardIterator& compare_to) const
    {
        return before_beginning != compare_to.before_beginning ||
                base_t::current != compare_to.current;
    }

};


// TODO: Rename or put into namespace to isolate this specifically for NODE iteration
template <class TValue, class TNodeTraits, class TBase>
struct ReverseIterator : public TBase
{
    typedef TBase base_t;
    typedef TNodeTraits traits_t;
    typedef typename base_t::node_type   node_type;
    typedef typename base_t::value_type  value_type;
    typedef typename base_t::node_pointer node_pointer;
    typedef typename base_t::node_handle_t node_handle_t;
    typedef typename base_t::node_allocator_t node_alloc_t;
    typedef ReverseIterator<TValue, TNodeTraits, TBase> iterator;

public:
    ReverseIterator(node_handle_t node, const traits_t& t) :
            base_t(node, t)
    {
    }

    ReverseIterator operator--()
    {
        node_type& c = base_t::lock_internal();

        this->current = base_t::traits.prev(c);

        base_t::unlock_internal();

        return *this;
    }

    ReverseIterator operator--(int)
    {
        ReverseIterator temp(*this);
        operator --();
        return temp;
    }

    // untested
    ReverseIterator& operator -=(int subtrahend)
    {
        while(subtrahend--) operator --();
        return *this;
    }

    // untested
    ReverseIterator operator -(int subtrahend) const
    {
        ReverseIterator temp(*this);

        return temp - subtrahend;
    }

    // untested
    int operator -(ReverseIterator subtrahend)
    {
        int counter = 0;

        while(subtrahend != *this)
        {
            counter++;
            ++subtrahend;
        }

        return counter;
    }
};


template <class TValue, class TNodeTraits, class TBase = InputIterator<TValue, TNodeTraits> >
struct BidirectionalIterator :
        public ReverseIterator<
            TValue,
            TNodeTraits,
            ForwardIterator<TValue, TNodeTraits, TBase> >
{
    typedef ReverseIterator<TValue, TNodeTraits, ForwardIterator<TValue, TNodeTraits> > base_t;
    typedef TNodeTraits traits_t;
    typedef typename base_t::node_type   node_type;
    typedef typename base_t::value_type  value_type;
    typedef typename base_t::node_pointer node_pointer;
    typedef typename base_t::node_handle_t node_handle_t;
    typedef typename base_t::node_allocator_t node_alloc_t;

    BidirectionalIterator(node_handle_t node, const traits_t& t) :
        base_t(node, t) {}
};

}}

}
