#pragma once

#include "../../platform.h"
#include <stdlib.h> // for size_t
//#include <memory> // for allocator_traits
#include "../traits/allocator_traits.h"

namespace estd {

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

#ifdef FEATURE_CPP_CONSTEXPR
        constexpr
#endif
        operator int() const { return 0; }
    };

    struct allocated_size_helper
    {
        NODATA_MOTIVATOR;
    };

    typedef T value_type;
    typedef T& reference;
    typedef T* handle_type;
    typedef T* pointer;
    typedef const void* const_void_pointer;
    typedef pointer handle_with_offset;
    typedef pointer handle_with_size;
    typedef size_t size_type;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    reference lock(handle_type h,
                   size_type pos,
                   size_type count) { return *(h + pos); }
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


// FIX: stand-in which has our additional locking/handle machanisms
// eventually have a proper one of these living in memory.h
template <class T>
struct experimental_std_allocator : public ::std::allocator<T>
{
    typedef ::std::allocator<T> base_t;

    typedef typename base_t::pointer handle_type;
    typedef handle_type handle_with_size;
    typedef handle_type handle_with_offset;
    typedef typename estd::nothing_allocator<T>::lock_counter lock_counter;
    typedef const void* const_void_pointer;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    static T& lock(handle_type h, size_t pos, size_t count) { return *(h + pos); }
    void unlock(handle_type) {}
};





#if __cplusplus >= 201103L
// semi-kludgey, a way to shoehorn in existing std::allocator using our extended
// locking mechanism.  Eventually use type_traits + SFINAE to auto deduce non-
// existing handle_type, etc.
template<class T>
struct allocator_traits< ::std::allocator<T> >
        : public ::std::allocator_traits< ::std::allocator<T > >
{
    typedef ::std::allocator_traits< ::std::allocator<T > > base_t;

    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::pointer handle_type;
    typedef typename base_t::value_type value_type;
    typedef typename base_t::size_type size_type;
    typedef handle_type handle_with_offset;
    typedef handle_type handle_with_size;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    typedef typename nothing_allocator<T>::lock_counter lock_counter;

    value_type& lock(allocator_type& a, handle_type h, size_type pos, size_type count)
    {
        return *(pos + count);
    }

    void unlock(allocator_type& a, handle_type h) {}
};
#endif


}
