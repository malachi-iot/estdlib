/**
 * @file
 */
#pragma once

#define FEATURE_ESTD_LIST_BEFORE_BEGINNING

#include "../type_traits.h"
#include "../internal/value_evaporator.h"

namespace estd {

namespace internal {

// If T is an empty struct, this will resolve to a temporary
// Otherwise, it resolves to a reference
// Almost identical to value_evaporator except this uses references specifically.
// 90% sure we could in fact use value_evaporator
template <class T, class Enable>
struct reference_or_temporary_provider;

/*
template <class T, class Enable = typename estd::enable_if<sizeof(T) != 0>::type >
struct reference_or_temporary_provider
{

};

template <class T, class Enable = typename estd::enable_if<sizeof(T) == 0>::type >
struct reference_or_temporary_provider<T, Enable>
{

};
*/

template <class T>
struct reference_or_temporary_provider<T, typename estd::enable_if<sizeof(T) != 0>::type>
{

};


template <class T>
struct reference_or_temporary_provider<T, typename estd::enable_if<sizeof(T) == 0>::type>
        : estd::experimental::temporary_provider<T>
{

};


// FIX: stateful traits in general is kinda suspect (statefulness should be
// tracked separately in allocator probably).  But until that point, manage
// as best we can - treating non-stateful traits appropriately
template <class TNodeTraits>
struct node_traits_evaporator
{
    typedef typename estd::remove_reference<TNodeTraits>::type traits_type;

protected:
    // FIX: Pretty sure we want this to be a reference, not a value
    TNodeTraits traits;

public:
    node_traits_evaporator(TNodeTraits traits) : traits(traits) {}
};

template <class T, T default_value = T()>
struct default_tester {};

template <class T, bool is_present>
struct reference_evaporator;

template <class T>
struct reference_evaporator<T, false>
{
    typedef typename estd::remove_reference<T>::type value_type;
    typedef T& reference;
    // FIX: Get proper name for this, this will be a value or ref depending on how things
    // got evaporated
    typedef value_type ref_type_exp;

    value_type value() { return value_type{}; }
    const value_type value() const { return value_type{}; }

    reference_evaporator(reference) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_evaporator(value_type&&) {}
#endif
};


template <class T>
struct reference_evaporator<T, true>
{
    typedef typename estd::remove_reference<T>::type value_type;
    typedef T& reference;
    // FIX: Get proper name for this, this will be a value or ref depending on how things
    // got evaporated
    typedef reference ref_type_exp;

    reference m_value;

    reference value() { return m_value; }
    const reference value() const { return m_value; }

    reference_evaporator(reference value) : m_value(value) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_evaporator(value_type&& value) :
        //m_value(std::move(value))
        m_value(value)
    {}
#endif
};


// Because of https://stackoverflow.com/questions/5687540/non-type-template-parameters
// we can't use this here (we are pushing struct thru T)
template <class T,
        class TValue = typename estd::remove_reference<T>::type,
        TValue default_value = TValue()>
struct reference_evaporator_old :
        estd::internal::value_evaporator<
            T&,
            //sizeof(TValue) == 0,
            false,
            TValue, default_value>
{
    typedef TValue value_type;
    typedef estd::internal::value_evaporator<
            T&,
            //sizeof(value_type) == 0,
            false,
            value_type,
            default_value>
            base_type;

    typedef T& reference;

    reference_evaporator_old(reference r) : base_type(r, true)
    {}
};


}

// adapted from util.embedded version
template <class TValue, class TNodeTraits>
struct InputIterator : //internal::node_traits_evaporator<TNodeTraits>
        //internal::reference_evaporator<TNodeTraits>
        internal::reference_evaporator<
                TNodeTraits,
                !estd::is_empty<
                        typename estd::remove_reference<TNodeTraits>::type>
                        ::value >
        /*
        estd::internal::value_evaporator<
            TNodeTraits&,
            sizeof(typename estd::remove_reference<TNodeTraits>::type) == 0> */
{
    //typedef internal::node_traits_evaporator<TNodeTraits> base_type;
    //typedef internal::reference_evaporator<TNodeTraits> base_type;
    typedef internal::reference_evaporator<
            TNodeTraits,
            !estd::is_empty<
                    typename estd::remove_reference<TNodeTraits>::type>
                    ::value >
            base_type;

    /*
    typedef estd::internal::value_evaporator<
            TNodeTraits&,
            sizeof(typename estd::remove_reference<TNodeTraits>::type) == 0> base_type; */

    //typedef typename base_type::traits_type traits_t;
    typedef typename base_type::value_type traits_t;
    typedef typename base_type::ref_type_exp traits_ref_type;

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

    traits_ref_type get_traits() { return base_type::value(); }
    const traits_ref_type get_traits() const { return base_type::value(); }
    /*
    traits_t& get_traits() { return base_type::traits; }
    const traits_t& get_traits() const { return base_type::traits; } */

    node_type& lock_internal()
    {
        return get_traits().lock(current);
    }

    void unlock_internal()
    {
        get_traits().unlock(current);
    }

public:
    InputIterator(node_handle_t node, const traits_t& traits) :
    // FIX: clean up this brute force const removal
        base_type((traits_ref_type)traits),
        current(node)
    {}

    InputIterator(const InputIterator& copy_from) :
        base_type((traits_ref_type)copy_from.get_traits()),
        current(copy_from.current)
    {

    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    InputIterator(InputIterator&& move_from) :
        base_type(std::move(move_from.get_traits())),
        current(std::move(move_from.current))
    {}
#endif

    //~InputIterator() {}

    static nv_reference lock(traits_ref_type a, node_handle_t& handle_to_lock)
    {
        node_type& n = a.lock(handle_to_lock);
        return traits_t::value(n);
    }

    // non standard handle-based mem helpers
    nv_reference lock() { return lock(get_traits(), current); }

    void unlock()
    {
        get_traits().unlock(current);
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
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
    // special mode for iterator representing before beginning.  we use NULL (invalid())
    // to designate end, so we need this to disambiguate
    // consider this experimental and unfortunately it fattens up the iterator as well
    // TODO: optimize this with some 'magic' values for this->current, possibly acquired
    // through a traits
    bool before_beginning;
#endif

    typedef TBase base_t;
    typedef typename base_t::traits_t   traits_t;
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

    ForwardIterator(node_handle_t node, const traits_t& t
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
        ,bool before_beginning = false
#endif
        ) :
            base_t(node, t)
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
            ,before_beginning(before_beginning)
#endif
    {
    }


    ForwardIterator(const ForwardIterator& copy_from) :
        base_t(copy_from)
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
        ,before_beginning(copy_from.before_beginning)
#endif
    {

    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    ForwardIterator(ForwardIterator&& move_from) :
        base_t(std::move(move_from))
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
        ,before_beginning(move_from.before_beginning)
#endif
    {

    }
#endif


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
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
        if(before_beginning)
        {
            before_beginning = false;
            return *this;
        }
#endif

        node_type& c = base_t::lock_internal();

        node_handle_t new_current = base_t::get_traits().next(c);

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
        return
#ifdef FEATURE_ESTD_LIST_BEFORE_BEGINNING
            before_beginning == compare_to.before_beginning &&
#endif
            base_t::current == compare_to.current;
    }

    bool operator!=(const ForwardIterator& compare_to) const
    {
        return !operator ==(compare_to);
    }

    ForwardIterator& operator=(const ForwardIterator& copy_from)
    {
        new (this) ForwardIterator(copy_from);
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    ForwardIterator& operator=(ForwardIterator&& move_from)
    {
        new (this) ForwardIterator(std::move(move_from));
    }
#endif
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

        this->current = base_t::get_traits().prev(c);

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
