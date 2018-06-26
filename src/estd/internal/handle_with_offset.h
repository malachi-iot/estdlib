#pragma once

#include <cstddef> // for size_t
#include "impl/handle_desc.h"

namespace estd { namespace internal {

// beware this will permit different handles themselves to do math operations
// on each other.  Obviously undefined, but it seems no more dangerous than
// stock standard iterator math
template <typename size_t>
class handle_with_offset_base
{
protected:
    size_t m_offset;

    handle_with_offset_base(size_t m_offset) :
            m_offset(m_offset) {}

public:
    typedef size_t size_type;

    // TODO: Make allocator version which does bounds checking
    // TODO: phase out increment if we can in favor of += operator
    void increment(size_type count = 1)
    {
        m_offset += count;
    }

    inline handle_with_offset_base& operator+=(size_type summand)
    {
        m_offset += summand;
        return *this;
    }

    inline handle_with_offset_base& operator-=(size_type subtrahend)
    {
        m_offset -= subtrahend;
        return *this;
    }

    ptrdiff_t operator-(const handle_with_offset_base& subtrahend) const
    {
        return m_offset - subtrahend.m_offset;
    }

    // NOTE: be mindful that these comparisons presume either:
    // a) a matching handle, if one exists
    // b) a non-matching handle whose non-matching nature doesn't matter, if one exists
    // c) no handle at all (dummy handle, like fixed allocator)
    // if none of these are the case, be sure to overload these operators
    // in inherited classes
    bool operator >(const handle_with_offset_base& compare_to) const
    {
        return m_offset > compare_to.m_offset;
    }

    bool operator==(const handle_with_offset_base& compare_to) const
    {
        return m_offset == compare_to.m_offset;
    }
};

// typically this will only be bool for THandle, but feel free
// to specialize for other conditions if you need to
template<class THandle, typename size_t = std::size_t>
class handle_with_only_offset;

template<typename size_t>
class handle_with_only_offset<bool, size_t> :
        public handle_with_offset_base<size_t>
{
    typedef handle_with_offset_base<size_t> base_t;

public:
    handle_with_only_offset(size_t offset) :
            base_t(offset) {}

    // TODO: Assert that incoming h == true
    handle_with_only_offset(bool h, size_t offset) :
            base_t(offset) {}

    size_t offset() const { return base_t::m_offset; }

    bool handle() const
    {
        return true;
    }
};


template<class THandle, typename size_type = std::size_t>
class handle_with_offset :
        public handle_with_offset_base<size_type>
{
    typedef handle_with_offset_base<size_type> base_t;

    THandle m_handle;

public:
    handle_with_offset(THandle h, size_type offset) :
            m_handle(h),
            base_t(offset) {}

    std::ptrdiff_t offset() const { return base_t::m_offset; }

    THandle handle() const { return m_handle; }

    bool operator==(const handle_with_offset& compare_to) const
    {
        return m_handle == compare_to.m_handle && offset() == compare_to.offset();
    }
};


template<class TPointer, typename size_t = std::size_t>
class handle_with_offset_raw
{
    TPointer m_pointer;

public:
    handle_with_offset_raw(TPointer p) : m_pointer(p) {}

    std::ptrdiff_t offset() const { return m_pointer; }

    TPointer handle() const { return m_pointer; }

    bool operator==(const handle_with_offset_raw& compare_to) const
    {
        return m_pointer == compare_to.m_pointer;
    }

    // TODO: Make allocator version which does bounds checking
    void increment(size_t count = 1)
    {
        m_pointer += count;
    }

    inline handle_with_offset_raw& operator +=(size_t summand)
    {
        m_pointer += summand;
        return *this;
    }

    inline std::ptrdiff_t operator -(const handle_with_offset_raw& subtrahend) const
    {
        return m_pointer - subtrahend.m_pointer;
    }
};



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

protected:
    // Used because for stateless, this is actually NOT a ref
    typedef typename base_t::allocator_ref allocator_ref;

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
        base_t::get_allocator().cunlock(h.handle());
    }

    operator value_type()
    {
        // copies it - beware, some T we don't want to copy!
        value_type retval = lock();

        unlock();

        return retval;
    }

    operator value_type() const
    {
        // copies it - beware, some T we don't want to copy!
        value_type retval = clock();

        cunlock();

        return retval;
    }


    bool operator >(const value_type& compare_to) const
    {
        bool greater_than = clock() > compare_to;
        cunlock();
        return greater_than;
    }


    bool operator ==(const value_type& compare_to) const
    {
        const value_type& v = clock();

        bool result = v == compare_to;

        cunlock();

        return result;
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
    typename allocator_traits::lock_counter lock_counter;

    handle_with_offset h;

    reference lock()
    {
        lock_counter++;
        reference retval = allocator_traits::lock(a, h);
        p = &retval;
        return retval;
    }


    const value_type& clock_experimental() const
    {
        allocator_traits::clock_experimental(a, h);
    }


    void unlock()
    {
        lock_counter--;
        if(lock_counter == 0)
            p = NULLPTR;
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
        if(lock_counter > 0)
            unlock();
    }

    operator reference()
    {
        return lock();
    }

    operator const value_type& () const
    {
        return clock_experimental();
    }

    operator value_type()
    {
        value_type v = lock();
        unlock();
        return v;
    }
};



}}
