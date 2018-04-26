#pragma once

#include "../../platform.h"
#include <stdlib.h> // for size_t
#include <memory> // for allocator_traits

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

}
