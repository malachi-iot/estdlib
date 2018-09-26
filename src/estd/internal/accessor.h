/**
 * @file
 */
#pragma once

// accessor is a special variety of allocator_descriptor which tracks
// handle and offset, but not specifically size.  Particularly useful
// when trying to substitute for a standard pointer

#include "handle_with_offset.h"

namespace estd {

namespace internal {

template <
        class TAllocator, bool is_stateful,
        class TTraits = estd::allocator_traits<TAllocator> >
class accessor_shared : public impl::allocator_descriptor<TAllocator&, is_stateful>
{
public:
    typedef impl::allocator_descriptor<TAllocator&, is_stateful> base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;
    //typedef typename base_t::allocator_traits allocator_traits;
    typedef TTraits allocator_traits;

    static bool CONSTEXPR is_locking = internal::has_locking_tag<allocator_type>::value;
    static bool CONSTEXPR is_pinned = internal::has_is_pinned_tag_exp_typedef<allocator_type>::value;

protected:
    // Used because for stateless, this is actually NOT a ref
    typedef typename base_t::allocator_ref allocator_ref;

    typedef typename estd::conditional<is_locking, value_type, value_type&>::type lockless_value_type;
    typedef typename estd::conditional<is_pinned, value_type&, value_type>::type ref_when_pinned;
    typedef typename estd::conditional<is_pinned, const value_type&, value_type>::type const_ref_when_pinned;

    handle_with_offset h;

    template <class TAccessor>
    TAccessor& assign(const value_type& assign_from)
    {
        value_type& value = lock();

        value = assign_from;

        unlock();

        return * (TAccessor*) this;
    }

public:
    accessor_shared(const accessor_shared& copy_from) :
        // FIX: Repair this const-dropping trick
        base_t((allocator_ref)copy_from.get_allocator()),
        h(copy_from.h) {}

    accessor_shared(allocator_type& a, const handle_with_offset& h) :
        base_t(a),
        h(h) {}

    accessor_shared(const handle_with_offset& h) :
        h(h) {}

    const handle_with_offset& h_exp() const { return h; }

    handle_with_offset& h_exp() { return h; }

    value_type& lock()
    {
        allocator_ref a = base_t::get_allocator();

        return allocator_traits::lock(a, h);
    }

    void unlock()
    {
        allocator_ref a = base_t::get_allocator();

        allocator_traits::unlock(a, h.handle());
    }


    const value_type& clock() const
    {
        return allocator_traits::clock(base_t::get_allocator(), h);
    }

    void cunlock() const
    {
        allocator_traits::cunlock(base_t::get_allocator(), h.handle());
    }

    operator ref_when_pinned()
    {
        // copies it - beware, some T we don't want to copy!
        ref_when_pinned retval = lock();

        unlock();

        return retval;
    }

    operator const_ref_when_pinned() const
    {
        // when not pinned memory, copies it -
        // beware, some T we don't want to copy!
        const_ref_when_pinned retval = clock();

        cunlock();

        return retval;
    }

    // EXPERIMENTAL: to try to accomodate scenarios where value_type is a *
    // be careful because in non-pinned scenarios this will be a temporary/copy
    // though a copy of a pointer type, so probably OK
    ref_when_pinned operator->()
    {
        // copies it - beware, some T we don't want to copy!
        ref_when_pinned retval = lock();

        unlock();

        return retval;
    }


    // EXPERIMENTAL: to try to accomodate scenarios where value_type is a *
    const_ref_when_pinned operator->() const
    {
        // copies it - beware, some T we don't want to copy!
        const_ref_when_pinned retval = clock();

        cunlock();

        return retval;
    }


    bool operator >(const value_type& compare_to) const
    {
        bool greater_than = clock() > compare_to;
        cunlock();
        return greater_than;
    }


    bool operator <(const value_type& compare_to) const
    {
        bool is_less_than = clock() < compare_to;
        cunlock();
        return is_less_than;
    }


    bool operator ==(const value_type& compare_to) const
    {
        const value_type& v = clock();

        bool result = v == compare_to;

        cunlock();

        return result;
    }

    bool operator !=(const value_type& compare_to) const
    {
        bool is_not_equal = clock() != compare_to;
        cunlock();
        return is_not_equal;
    }
};


template <class TAllocator>
class accessor : public accessor_shared<TAllocator, true >
{
    typedef accessor_shared<TAllocator, true > base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef accessor this_t;
    typedef typename base_t::value_type value_type;

public:
    accessor(TAllocator& a, const typename base_t::handle_with_offset& h) :
        base_t(a, h) {}

    // NOTE: Making this non-const because get_allocator() needs to be non const
    // as this accessor can peer in and change stuff (legitimate non-const locking)
    accessor(const accessor& copy_from) : base_t(copy_from)
    {}

    accessor& operator=(const value_type& assign_from)
    {
        return base_t::template assign<accessor>(assign_from);
    }
};


template <class TAllocator>
class accessor_stateless : public accessor_shared<TAllocator, false>
{
    typedef accessor_shared<TAllocator, false> base_t;
    typedef typename base_t::value_type value_type;
    typedef typename base_t::handle_with_offset handle_with_offset;

public:
    accessor_stateless(const handle_with_offset& h) :
        base_t(h) {}

    accessor_stateless(const TAllocator& a, const handle_with_offset& h) :
        base_t(h) {}

    accessor_stateless(const accessor_stateless& copy_from) :
        base_t(copy_from.h) {}

    accessor_stateless& operator=(const value_type& assign_from)
    {
        return base_t::template assign<accessor_stateless>(assign_from);
    }
};


}

namespace experimental {

// TODO: Interrogate TAllocator::is_locking_tag to decide which accessor to use

// nonlocking:
// we still go through the 'lock' paradigm for handle -> pointer translation, but
// there's no necessity to unlock and we don't expect the pointer to ever move
template <class TAllocator>
class stateful_nonlocking_accessor
{
    TAllocator& a;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_traits::reference reference;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;
    handle_with_offset h;

public:
    stateful_nonlocking_accessor(TAllocator& a) : a(a) {}

    operator reference()
    {
        return allocator_traits::lock(a, h);
    }
};

// locking means underlying memory could in fact moved if unlocked
// NOTE: not yet determined if allocator itself is/should be maintaining
// lock counters itself (probably yes)
template <class TAllocator>
class stateful_locking_accessor
{
    TAllocator& a;
    typedef estd::allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_traits::reference reference;
    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;

    // FIX: Undecided if we should track 'is locked' status by p, lock_counter
    // or inbuilt allocator counter
    pointer p;
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    typename allocator_traits::lock_counter lock_counter;
#endif

    handle_with_offset h;

    reference lock()
    {
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
        lock_counter++;
#endif
        reference retval = allocator_traits::lock(a, h);
        p = &retval;
        return retval;
    }


    const value_type& clock() const
    {
        allocator_traits::clock(a, h);
    }


    void unlock()
    {
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
        lock_counter--;
        if(lock_counter == 0)
            p = NULLPTR;
#endif
        allocator_traits::unlock(a, h.handle());
    }

public:
    stateful_locking_accessor(TAllocator& a, handle_with_offset h)
            :
            a(a),
            p(NULLPTR),
            h(h)
    {}

    ~stateful_locking_accessor()
    {
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
        if(lock_counter > 0)
            unlock();
#endif
    }

    operator reference()
    {
        return lock();
    }

    operator const value_type& () const
    {
        return clock();
    }

    operator value_type()
    {
        value_type v = lock();
        unlock();
        return v;
    }
};

}

}
