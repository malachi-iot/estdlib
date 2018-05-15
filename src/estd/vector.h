#pragma once

#include <stdlib.h>
#include "memory.h"
#include "internal/dynamic_array.h"
#include "allocators/fixed.h"

namespace estd {

template <class T, class Allocator = std::allocator<T > >
class vector : public internal::dynamic_array<Allocator>
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

private:
    typedef internal::dynamic_array<Allocator> base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::handle_with_offset handle_with_offset;

    typedef allocator_traits<allocator_type> allocator_traits_t;

public:
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
    vector() = default;
#else
    vector() {}
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<value_type> initlist) : base_t(initlist) {}
#endif

    typedef typename base_t::size_type size_type;
    typedef typename base_t::accessor accessor;
    typedef typename base_t::iterator iterator;
};


namespace layer1 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N > >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N > > base_t;

public:
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
    vector() = default;
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif
};

};


namespace layer2 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > > base_t;

public:
    //vector(T* underlying_buffer) : base_t(underlying_buffer) {}

#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
    vector() = default;
#endif

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif
};

}

}
