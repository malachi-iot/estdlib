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

    static handle_type allocate(allocator_type& a, size_type n)
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


// Non-standard and doesn't make a difference for some compilers
// https://stackoverflow.com/questions/621616/c-what-is-the-size-of-an-object-of-an-empty-class
#define NODATA_MOTIVATOR    char NO_DATA[0]

// Non standard
template <class T>
struct nothing_allocator
{
    NODATA_MOTIVATOR;

    struct lock_counter
    {
        NODATA_MOTIVATOR;

        lock_counter& operator++() {return *this;}
        lock_counter& operator--() {return *this;}
        lock_counter& operator++(int) {return *this;}
        lock_counter& operator--(int) {return *this;}

        CONSTEXPR operator int() const { return 0; }
    };

    struct allocated_size_helper
    {
        NODATA_MOTIVATOR;
    };

    typedef T value_type;
    typedef T& reference;
    typedef T* handle_type;
    typedef T* pointer;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    reference lock(handle_type h) { return *h; }
    void unlock(handle_type h) {}

    // Don't want this here, but needed so far for ~dynamic_array, since
    // current estd::string specifies nothing_allocator at present
    void deallocate(handle_type h, int size) {}

    // allocate also will always fail
    handle_type allocate(int size) { return invalid(); }

    // reallocate also will always fail
    handle_type reallocate(handle_type h, int size) { return invalid(); }

    size_t max_size() const { return 0; }

    // return size used by handle, with help from size helper
    size_t allocated_size(handle_type h, const allocated_size_helper& ash)
    {
        return 0;
    }
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
    typedef typename allocator_type::allocated_size_helper ash;
    typedef std::size_t size_type;

private:
    // hopefully someday we can lean on allocator to tell us this
    size_type m_capacity;

    ash m_capacity_exp;

    size_type m_size;

protected:
    allocator_type allocator;
    typename allocator_type::lock_counter lock_counter;
    handle_type handle;

    T* lock()
    {
        lock_counter++;
        return &allocator.lock(handle);
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
            handle = allocator.allocate(new_cap);
        else
            handle = allocator.reallocate(handle, new_cap);

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

// definitely experimental.  perhaps we can stuff this into would-be dynamic allocated
// things to make them into fixed-allocated entities

template <size_t size>
class fixed_allocator
{
    uint8_t buffer[size];

public:

};

}

}
