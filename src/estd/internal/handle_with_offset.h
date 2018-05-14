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



template <class TAllocator>
class accessor_stateful_base
{
public:
    typedef TAllocator allocator_type;
    typedef allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

private:
    allocator_type& a;

protected:
    handle_with_offset h;

    typedef allocator_type& allocator_ref;

public:
    accessor_stateful_base(allocator_type& a, const handle_with_offset& h) :
        a(a),
        h(h) {}

    allocator_type& get_allocator() const { return a; }
};



template <class TAllocator>
class accessor_stateless_base
{
public:
    typedef TAllocator allocator_type;
    typedef allocator_traits<TAllocator> allocator_traits;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

protected:
    handle_with_offset h;

    typedef allocator_type allocator_ref;

public:
    accessor_stateless_base(const handle_with_offset& h) : h(h) {}

#ifdef FEATURE_CPP_MOVE_SEMANTIC
    allocator_type&& get_allocator() const { return allocator_type(); }
#else
    allocator_type get_allocator() const { return allocator_type(); }
#endif
};

template <class TAccessorBase>
class accessor_shared : public TAccessorBase
{
public:
    typedef TAccessorBase base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::value_type value_type;
    typedef typename base_t::handle_with_offset handle_with_offset;
    typedef typename base_t::allocator_traits allocator_traits;

protected:
    typedef typename base_t::allocator_ref allocator_ref;

    void unlock() const
    {
        // FIX: temporarily not unlocking anything since const lock/unlock still a pain
        // and we aren't actually doing anything during unlock yet
    }

    template <class TAccessor>
    TAccessor& assign(const value_type& assign_from)
    {
        value_type& value = lock();

        value = assign_from;

        unlock();

        return * (TAccessor*) this;
    }

public:
    accessor_shared(allocator_type& a, const handle_with_offset& h) :
        base_t(a, h) {}

    accessor_shared(const handle_with_offset& h) :
        base_t(h) {}

    const handle_with_offset& h_exp() const { return base_t::h; }

    handle_with_offset& h_exp() { return base_t::h; }

    value_type& lock()
    {
        allocator_ref a = base_t::get_allocator();

        return allocator_traits::lock(a, base_t::h);
    }

    void unlock()
    {
        allocator_ref a = base_t::get_allocator();

        allocator_traits::unlock(a, base_t::h.handle());
    }


    const value_type& clock_experimental() const
    {
        allocator_ref a = base_t::get_allocator();

        return allocator_traits::clock_experimental(a, base_t::h);
    }

    operator value_type()
    {
        // copies it - beware, some T we don't want to copy!
        value_type retval = lock();

        unlock();

        return retval;
    }


    bool operator ==(const value_type& compare_to) const
    {
        const value_type& v = clock_experimental();

        bool result = v == compare_to;

        unlock();

        return result;
    }
};


template <class TAllocator>
class accessor : public accessor_shared<accessor_stateful_base<TAllocator> >
{
    typedef accessor_shared<accessor_stateful_base<TAllocator> > base_t;
    typedef accessor this_t;
    typedef typename base_t::value_type value_type;

public:
    accessor(TAllocator& a, const typename base_t::handle_with_offset& h) :
        base_t(a, h) {}

    accessor& operator=(const value_type& assign_from)
    {
        return base_t::template assign<accessor>(assign_from);
    }
};


template <class TAllocator>
class accessor_stateless : public accessor_shared<accessor_stateless_base<TAllocator> >
{
    typedef accessor_shared<accessor_stateless_base<TAllocator> > base_t;
    typedef typename base_t::value_type value_type;
    typedef typename base_t::handle_with_offset handle_with_offset;

public:
    accessor_stateless(const handle_with_offset& h) :
        base_t(h) {}

    accessor_stateless(const TAllocator& a, const handle_with_offset& h) :
        base_t(h) {}

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
