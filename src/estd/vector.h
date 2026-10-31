#pragma once

#include <stdlib.h>
#include "memory.h"
#include "internal/dynamic_array.h"
#include "allocators/fixed.h"
#include "span.h"

namespace estd {

template <class T, class Allocator
#ifdef FEATURE_STD_MEMORY
          = std::allocator<T>
#endif
          >
using vector = internal::dynamic_array<
    internal::impl::dynamic_array<
        Allocator, internal::impl::default_dynamic_array_policy>>;


namespace layer1 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::layer1::allocator<T, N>>
{
    using base_type = estd::vector<T, estd::layer1::allocator<T, N>>;

public:
    using typename base_type::reference;
    using typename base_type::const_reference;

    ESTD_CPP_DEFAULT_CTOR(vector)

    constexpr vector(std::initializer_list<T> initlist) : base_type(initlist) {}

    // Because layer1 is *always* a simple fixed buffer, enable data() call here
    // Also, for that same reason, lock/clock are no-ops, so we can call them without
    // a corresponding free operation
    T* data() { return base_type::lock(); }
    const T* data() const { return base_type::clock(); }

    // +++ DEBT: Workaround for https://github.com/malachi-iot/estdlib/issues/97
    reference at(unsigned i)
    {
        return base_type::at(i).lock();
    }

    const_reference at(unsigned i) const
    {
        return base_type::at(i).clock();
    }
    // ---
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
    using base_type = estd::vector<T,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        estd::layer3::allocator<T>
#else
        estd::internal::single_fixedbuf_runtimesize_allocator<T>
#endif
        >;
    using typename base_type::impl_type;
    using typename base_type::allocator_type;
    typedef typename allocator_type::size_type size_type;

public:
    constexpr vector(T* underlying_buffer, size_type n) :
        base_type(span<T>(underlying_buffer, n)) {}

    template <size_type N>
    constexpr vector(T (&buffer)[N]) :
        base_type(span<T, N>(buffer)) {}

#ifdef FEATURE_STD_INITIALIZER_LIST
    constexpr vector(std::initializer_list<T> init_list) : base_type(init_list) {}
#endif

    // Because layer3 is *always* a simple pointer, enable data() call here
    T* data() { return base_type::lock(); }
    constexpr const T* data() const { return base_type::clock(); }
};

}

}
