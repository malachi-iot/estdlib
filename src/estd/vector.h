#pragma once

#include <stdlib.h>
#include "memory.h"
#include "internal/dynamic_array.h"
#include "allocators/fixed.h"

namespace estd {

template <class T, class Allocator
#ifdef FEATURE_STD_MEMORY
          = std::allocator<T >
#endif
          >
class vector : public internal::dynamic_array<internal::impl::dynamic_array<Allocator, void > >
{
public:
    typedef T value_type;

private:
    typedef internal::dynamic_array<internal::impl::dynamic_array<Allocator, void > > base_t;

protected:
    template <class TImplParam>
    vector(const TImplParam& p) : base_t(p) {}

public:
    explicit vector(Allocator& alloc) : base_t(alloc) {}

#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
    vector() = default;
#else
    vector() {}
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<value_type> initlist) : base_t(initlist) {}
#endif
};


namespace layer1 {

template <class T, size_t N>
class vector : public estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer1::allocator<T, N>
#else
        estd::internal::single_fixedbuf_allocator<T, N >
#endif
        >
{
    typedef estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer1::allocator<T, N>
#else
        estd::internal::single_fixedbuf_allocator<T, N >
#endif
    > base_t;

public:
    typedef typename base_t::reference reference;
    typedef typename base_t::const_reference const_reference;

#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
    vector() = default;
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif

    // Because layer1 is *always* a simple fixed buffer, enable data() call here
    // Also, for that same reason, lock/clock are no-ops, so we can call them without
    // a corresponding free operation
    T* data() { return base_t::lock(); }
    const T* data() const { return base_t::clock(); }
};

}


namespace layer2 {

// Relies on an underlying T* but is expected to always have a specific constant size
template <class T, size_t N>
class vector : public estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer2::allocator<T, N>
#else
        estd::internal::single_fixedbuf_allocator<T, N, T* >
#endif
        >
{
    typedef estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer2::allocator<T, N>
#else
        estd::internal::single_fixedbuf_allocator<T, N, T* >
#endif
    > base_t;
    typedef typename base_t::size_type size_type;

public:
    vector(T* underlying_buffer, size_type initial_size = 0) :
            base_t(underlying_buffer)
    {
        base_t::impl().size(initial_size);
    }

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif

    // Because layer2 is *always* a simple pointer, enable data() call here
    T* data() { return base_t::lock(); }
    const T* data() const { return base_t::clock(); }
};

}


namespace layer3 {

template <class T>
class vector : public estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer3::allocator<T>
#else
        estd::internal::single_fixedbuf_runtimesize_allocator<T>
#endif
        >

{
    typedef estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer3::allocator<T>
#else
        estd::internal::single_fixedbuf_runtimesize_allocator<T>
#endif
        > base_t;
    typedef typename base_t::impl_type impl_type;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::InitParam init_t;
    typedef typename allocator_type::size_type size_type;

public:
    vector(T* underlying_buffer, size_type n) :
            base_t(init_t(underlying_buffer, n)) {}

    template <size_type N>
    vector(T (&buffer)[N]) :
            base_t(init_t(buffer, N)) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif

    // Because layer3 is *always* a simple pointer, enable data() call here
    T* data() { return base_t::lock(); }
    const T* data() const { return base_t::clock(); }
};

}

}
