#pragma once

#include "../platform.h"
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
struct allocator_traits
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef typename TAllocator::pointer        pointer;
    typedef size_t                              size_type;

    // non-standard, for handle based scenarios
    typedef typename TAllocator::handle_type    handle_type;

    typedef typename allocator_type::const_void_pointer     const_void_pointer;

    static handle_type allocate(allocator_type& a, size_type n)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }

    // NOTE: Consider strongly putting nonstandard lock and unlock in here,
    // to accomodate handle-based memory

    static pointer lock(allocator_type& a, handle_type h)
    {
        return h;
    }


    static void unlock(allocator_type& a, handle_type h) {}
};


template <class TPtr> struct pointer_traits;
template <class T> struct pointer_traits<T*>;


// Non standard
struct nothing_allocator
{
    struct lock_counter
    {
        lock_counter& operator++() {return *this;}
        lock_counter& operator--() {return *this;}
        lock_counter& operator++(int) {return *this;}
        lock_counter& operator--(int) {return *this;}

        CONSTEXPR operator int() const { return 0; }
    };

    typedef void* handle_type;
    typedef void* pointer;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    pointer lock(handle_type h) { return h; }
    void unlock(handle_type h) {}

    // Don't want this here, but needed so far for ~dynamic_array, since
    // current estd::string specifies nothing_allocator at present
    void deallocate(handle_type h, int size) {}

    // allocate also will always fail
    handle_type allocate(int size) { return invalid(); }

    // reallocate also will always fail
    handle_type reallocate(handle_type h, int size) { return invalid(); }
};



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
// accounts for lock/unlock behaviors
template <class T, class TAllocator>
class dynamic_array
{
public:
    typedef T value_type;
    typedef TAllocator allocator_type;

    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::pointer pointer;
    typedef std::size_t size_type;

private:
    // hopefully someday we can lean on allocator to tell us this
    size_type m_capacity;

    size_type m_size;

protected:
    allocator_type allocator;
    typename allocator_type::lock_counter lock_counter;
    handle_type handle;

    T* lock()
    {
        lock_counter++;
        return (T*) allocator.lock(handle);
    }

    void unlock()
    {
        lock_counter--;
        allocator.unlock(handle);
    }

public:
    dynamic_array() :
            handle(allocator_type::invalid()), m_size(0)
    {}

    ~dynamic_array()
    {
        if(handle != allocator_type::invalid())
            allocator.deallocate(handle, m_capacity * sizeof(T));
    }


    size_type size() const { return m_size; }

    size_type capacity() const
    {
        if(handle == allocator_type::invalid()) return 0;

        return m_capacity;
    }

    void reserve( size_type new_cap )
    {
        if(handle == allocator_type::invalid())
            handle = allocator.allocate(new_cap * sizeof(T));
        else
            handle = allocator.reallocate(handle, new_cap * sizeof(T));

        m_capacity = new_cap;
    }

    void push_back(const T& value)
    {
        size_type cap = capacity();

        if(size() == cap)
        {
            reserve(cap + ((32 + sizeof(T)) / sizeof(T)));
        }

        T* v = lock();

        v[size()] = value;

        unlock();

        m_size++;
    }

protected:
    void _append(const T* buf, size_t len)
    {
        if(size() + len >= capacity())
        {
            reserve(size() + len + (32 + sizeof(T)) / sizeof(T));
        }

        T* raw = lock();

        memcpy(raw, buf, len);

        unlock();

        m_size += len;
    }
};

}

}
