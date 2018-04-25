#pragma once

#include "allocators/generic.h"
#include "allocators/fixed.h"
#include <memory.h>

namespace estd {

#ifdef UNUSED
// 100% generic allocator
template <class T>
struct Allocator
{
    typedef T value_type;
    typedef value_type* pointer;
    typedef void* void_pointer;
    typedef const void* const_void_pointer;

    // non-standard, for handle based scenarios
    typedef pointer handle_type;
};
#endif


template <class TAllocator>
struct allocator_traits;




// NOTE: May very well be better off using inbuilt version and perhaps extending it with
// our own lock mechanism
// NOTE: I erroneously made our burgeouning custom allocators not-value_type aware
template <class TAllocator>
struct allocator_traits
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef typename TAllocator::pointer        pointer;
    typedef size_t                              size_type;
    typedef value_type&                         reference; // deprecated in C++17 but relevant for us due to lock/unlock

    // non-standard, for handle based scenarios
    typedef typename TAllocator::handle_type    handle_type;
    typedef typename allocator_type::const_void_pointer     const_void_pointer;

    static handle_type allocate(allocator_type& a, size_type n, const_void_pointer hint = NULLPTR)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }


    static reference lock(allocator_type& a, handle_type h)
    {
        return a.lock(h);
    }

    static void unlock(allocator_type& a, handle_type h)
    {
        a.unlock(h);
    }

    static size_type max_size(const allocator_type& a)
    {
        // note that a.max_size is no longer required (though spec
        // strongly implies it's optionally permitted) in C++17, though
        // allocator_traits::max_size is not
        return a.max_size();
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class T, class... TArgs>
    static void construct(allocator_type& a, T* p, TArgs&&... args)
    {
        new (static_cast<void*>(p)) T(std::forward<TArgs>(args)...);
    }
#endif
};


template <class TPtr> struct pointer_traits;
template <class T> struct pointer_traits<T*>;



namespace experimental {
#ifdef UNUSED
template <class T, class TAllocator>
class unique_handle : protected typed_handle<T, TAllocator>
{
    typedef typed_handle<T, TAllocator> base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::allocator_traits_t allocator_traits_t;
    typedef T value_type;

    TAllocator a;

public:
    unique_handle(TAllocator& a) :
        base_t(allocator_traits_t::allocate(a, sizeof(value_type))),
        a(a) {}

    unique_handle() :
       // some allocators are not stateful
        base_t(allocator_traits_t::allocate(a, sizeof(value_type)))
    {
        static_assert(sizeof(TAllocator) == 0, "Only stateless allocator permitted here");
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    unique_handle(unique_handle&& uh) :
        base_t(uh.handle),
        a(uh.a)
    {

    }
#endif

    T& lock() { return base_t::lock(a); }
    void unlock() { return base_t::unlock(a); }
};


#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
template <class T, class TAllocator, class... TArgs>
unique_handle<T, TAllocator> make_unique_handle(TAllocator& a, TArgs&&...args)
{
    unique_handle<T, TAllocator> uh(a);

    T& val = uh.lock();

    allocator_traits<TAllocator>::construct(a, &val, args...);

    uh.unlock();

    return uh;
}

template <class T, class TAllocator, class... TArgs>
unique_handle<T, TAllocator> make_unique_handle(TArgs&&...args)
{
    static_assert(sizeof(TAllocator) == 0, "Only stateless allocator permitted here");
    static TAllocator a;
    unique_handle<T, TAllocator> uh(a);

    T& val = uh.lock();

    allocator_traits<TAllocator>::construct(a, &val, args...);

    uh.unlock();

    return uh;
}
#endif
#endif

}





namespace experimental {


template <class TAllocator>
class memory_range_base
{
public:
    typedef TAllocator allocator_t;

    typedef typename allocator_t::handle_type handle_type;
    typedef typename allocator_t::pointer pointer;

private:
    //allocator_t
    handle_type handle;

public:
    template <class T>
    T& lock()
    {
        //pointer p =
    }
};


// non standard base class for managing expanding/contracting arrays
// accounts for lock/unlock behaviors. Used for vector and string
// More or less 1:1 with vector
// and may get rolled back completely into vector at some point -
template <class TAllocator>
class dynamic_array
{
public:
    typedef TAllocator allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_traits<TAllocator>::size_type size_type;

private:
    size_type m_size;

protected:
    allocator_type allocator;
    typename allocator_type::lock_counter lock_counter;
    handle_with_size handle;

    value_type* lock()
    {
        lock_counter++;
        return &allocator.lock(handle);
    }

    void unlock()
    {
        lock_counter--;
        allocator.unlock(handle);
    }

    // internal method for auto increasing capacity based on pre-set amount
    void ensure_additional_capacity(size_type increase_by)
    {
        size_type cap = capacity();

        // TODO: assert increase_by is a sensible value
        // above 0 and less than ... something

        if(size() + increase_by >= cap)
        {
            // increase by as near to 32 bytes as is practical
            reserve(cap + increase_by + ((32 + sizeof(value_type)) / sizeof(value_type)));
        }
    }


    void raw_insert(value_type* a, value_type* to_insert_pos, const value_type* to_insert_value)
    {
        // NOTE: may not be very efficient (underlying allocator may need to realloc/copy etc.
        // so later consider doing the insert operation at that level)
        ensure_additional_capacity(1);

        // NOTE: this shall be all very explicit raw array operations.  Not resilient to other data structure
        size_type raw_typed_pos = (((to_insert_pos) - a) / sizeof(value_type));
        size_type remaining = size() - raw_typed_pos;

        memmove(to_insert_pos + 1, to_insert_pos, remaining * sizeof(value_type));
        *to_insert_pos = *to_insert_value;

        m_size++;
    }

public:
    dynamic_array() :
            handle(allocator_type::invalid()),
            m_size(0)
    {}

    ~dynamic_array()
    {
        if(handle != allocator_type::invalid())
            allocator.deallocate(handle);
    }

    allocator_type get_allocator() const
    {
        return allocator;
    }

    size_type size() const { return m_size; }

    size_type capacity() const
    {
        if(handle == allocator_type::invalid()) return 0;

        return allocator.size(handle);
    }

    void reserve( size_type new_cap )
    {
        if(handle == allocator_type::invalid())
            handle = allocator.allocate_ext(new_cap);
        else
            handle = allocator.reallocate_ext(handle, new_cap);
    }

    void push_back(const value_type& value)
    {
        ensure_additional_capacity(1);

        value_type* v = lock();

        v[size()] = value;

        unlock();

        m_size++;
    }

protected:
    void _append(const value_type* buf, size_type len)
    {
        ensure_additional_capacity(len);

        value_type* raw = lock();

        memcpy(raw + m_size, buf, len);

        unlock();

        m_size += len;
    }
};


}

}
