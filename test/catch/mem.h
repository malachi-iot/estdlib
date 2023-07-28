#pragma once

#include "estd/memory.h"
#include <estd/internal/container/traditional_accessor.h>
#include <estd/internal/container/iterator.h>
#include <cstddef> // for ptrdiff_t

// reference allocator for inbuild mechanisms.  basically a crummy test-only
// version of std new_allocator
template <class T>
class _allocator
{
public:
    // Would like to use this somehow to enable/disable auto-unlocking
    // for iterators
    static constexpr bool is_locking() { return false; }

#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    typedef typename estd::nothing_allocator<T>::lock_counter lock_counter;
#endif

    typedef T value_type;
    typedef value_type* pointer;
    typedef const void* const_void_pointer;
    typedef value_type* handle_type;
    typedef size_t size_type;

    typedef estd::internal::handle_with_offset_raw<pointer> handle_with_offset;
    typedef T& accessor;
    typedef estd::internal::locking_iterator<
    	_allocator,
        estd::internal::traditional_accessor<value_type> >
        iterator;

    // primarily for subscript/array operations
    class handle_with_offset_old
    {
        pointer loc;

    public:
        handle_with_offset_old(pointer loc) : loc(loc) {}

        T& lock() { return *loc; }

        void unlock() {}

        // prefix version
        handle_with_offset& operator++()
        {
            loc++;
            return *this;
        }


        handle_with_offset& operator--()
        {
            loc--;
            return *this;
        }

        bool operator==(const handle_with_offset& compare_to) const
        {
            return loc == compare_to.loc;
        }

        bool operator!=(const handle_with_offset& compare_to) const
        {
            return loc != compare_to.loc;
        }

        handle_with_offset& operator=(const T& value)
        {
            // will autolock and unlock if necessary
            *loc = value;
        }

        operator T&()
        {
            // will autolock and unlock if necessary, returning only a const copy
            return *loc;
        }

        static size_type max_size() { return 999999; }
    };

    static handle_type invalid() { return NULLPTR; }

    class handle_with_size
    {
        handle_type h;
        size_t m_size;

    public:
        // FIX: Having a 0 size should never be valid, but we have scenarios where
        // we need to track an invalid handle type (unallocated)
        handle_with_size(handle_type h, size_t size = 0) :
            h(h), m_size(size) {}

        size_t size() const { return m_size; }

        operator handle_type() const { return h; }
    };

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    T& lock(handle_type h, size_t pos = 0, size_t count = 0) { return *(h + pos); }

    T& lock(handle_with_offset h, size_t pos = 0, size_t count = 0)
    {
        return *(h.handle() + pos);
    }

    const T& clock(handle_type h, size_t pos = 0, size_t count = 0) const
    {
        return *(h + pos);
    }

    const T& clock(const handle_with_offset& h, size_t pos = 0, size_t count = 0) const
    {
        return *(h.handle() + pos);
    }

    void unlock(handle_type) { }

    void cunlock(handle_type) const {}
#endif

    pointer allocate(size_t size)
    {
        return (pointer) malloc(size * sizeof(T));
    }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    // consider merging this with regular allocate and letting optimizer shake out
    // pointer access
    handle_with_size allocate_ext(size_t size)
    {
        return handle_with_size(allocate(size), size);
    }


    size_t size(const handle_with_size& h) const
    {
        return h.size();
    }
#endif


    void deallocate(handle_type p, size_t size)
    {
        free(p);
    }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    void deallocate(handle_with_size h)
    {
        free(h);
    }
#endif


    handle_type reallocate(handle_type h, size_t size)
    {
        return (pointer) realloc(h, size);
    }


#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    handle_with_size reallocate_ext(handle_type h, size_t size)
    {
        return handle_with_size(reallocate(h, size), size);
    }
#endif


    // TODO: template this to work with both handle_with_size or not
    handle_with_offset offset(handle_type h, ptrdiff_t size) const
    {
        return handle_with_offset(h + size);
    }

    // DEBT: Pretty sloppy, just brute forcing the one that doesn't need locking anyway
    //pointer lock(estd::internal::traditional_accessor<T>& a)
    value_type& lock(const estd::internal::handle_with_offset_raw<pointer, size_t>& a) const
    {
        return *a.handle();
    }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_stateful() { return false; }

    static CONSTEXPR bool is_singular() { return false; }

    static CONSTEXPR bool has_size() { return false; }
#endif

    static unsigned max_size() { return 99999; }
};

