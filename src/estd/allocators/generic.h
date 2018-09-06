#pragma once

#include <stdlib.h> // for size_t
#include "../traits/allocator_traits.h"
#include "../internal/handle_with_offset.h"
#ifdef FEATURE_STD_MEMORY
#include <memory> // for std::allocator_traits and friends
#endif

namespace estd {

// Non standard
template <class T>
struct nothing_allocator
{
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    struct lock_counter
    {
        lock_counter& operator++() {return *this;}
        lock_counter& operator--() {return *this;}
        lock_counter& operator++(int) {return *this;}
        lock_counter& operator--(int) {return *this;}

#ifdef FEATURE_CPP_CONSTEXPR
        constexpr
#endif
        operator int() const { return 0; }
    };
#endif

    typedef T value_type;
    typedef T& reference;
    typedef T* handle_type;
    typedef T* pointer;
    typedef const void* const_void_pointer;
    typedef internal::handle_with_offset_raw<pointer> handle_with_offset;
    //typedef pointer handle_with_size;
    typedef size_t size_type;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_locking() { return false; }
#endif

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
};

#ifdef FEATURE_STD_MEMORY

// FIX: stand-in which has our additional locking/handle machanisms
// eventually have a proper one of these living in memory.h
template <class T>
struct experimental_std_allocator : public ::std::allocator<T>
{
    typedef ::std::allocator<T> base_t;

    typedef typename base_t::pointer handle_type;
    typedef handle_type handle_with_size;
    typedef typename estd::internal::handle_with_offset_raw<handle_type> handle_with_offset;
#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    typedef typename estd::nothing_allocator<T>::lock_counter lock_counter;
#endif
    typedef const void* const_void_pointer;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static T& lock(handle_type h, size_t pos, size_t count) { return *(h + pos); }
    void unlock(handle_type) {}

    static CONSTEXPR bool is_locking() { return false; }
#endif

    template <class TNew>
    experimental_std_allocator<TNew> rebind_experimental()
    {
        return experimental_std_allocator<TNew>();
    }
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
    typedef typename base_t::pointer pointer;
    typedef typename base_t::pointer handle_type;
    typedef typename base_t::value_type value_type;
    typedef typename base_t::size_type size_type;
    //typedef handle_type handle_with_offset;
    typedef estd::internal::handle_with_offset_raw<pointer> handle_with_offset;
    typedef handle_type handle_with_size;

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_stateful() { return false; }

    static CONSTEXPR bool is_locking() { return false; }

    static CONSTEXPR bool has_size() { return false; }

    static CONSTEXPR bool is_singular() { return false; }
#endif

    // SFINAE = eventually we can phase out this entire specialization for std::allocator
    static CONSTEXPR bool is_stateful_exp = false;
    static CONSTEXPR bool has_size_exp = false;
    static CONSTEXPR bool is_singular_exp = false;
    static CONSTEXPR bool is_locking_exp = false;
    static CONSTEXPR bool is_contiguous_exp = true;

#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    typedef typename nothing_allocator<T>::lock_counter lock_counter;
#endif

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static value_type& lock(allocator_type& a, handle_type h, size_type pos, size_type count)
    {
        return *(h + pos);
    }

    static void unlock(allocator_type& a, handle_type h) {}
#endif
};
#endif

#endif

}
