#pragma once

#include <cstddef> // for size_t

namespace estd { namespace internal {

template<typename size_t>
class handle_with_only_offset
{
    size_t m_offset;

public:
    handle_with_only_offset(size_t offset) : m_offset(offset) {}

    size_t offset() const { return m_offset; }
};


template<class THandle, typename size_type = std::size_t>
class handle_with_offset
{
    THandle m_handle;
    size_type m_offset;

public:
    handle_with_offset(THandle h, size_type offset) :
            m_handle(h),
            m_offset(offset) {}

    std::ptrdiff_t offset() const { return m_offset; }

    THandle handle() const { return m_handle; }

    bool operator==(const handle_with_offset& compare_to) const
    {
        return m_handle == compare_to.m_handle && offset() == compare_to.offset();
    }

    // TODO: Make allocator version which does bounds checking
    void increment(size_type count = 1)
    {
        m_offset += count;
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
