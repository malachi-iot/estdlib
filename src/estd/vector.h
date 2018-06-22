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

private:
    typedef internal::dynamic_array<Allocator> base_t;

protected:
    template <class TImplParam>
    vector(const TImplParam& p) : base_t(p) {}

public:
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
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, T* > >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, T* > > base_t;

public:
    vector(T* underlying_buffer) : base_t(underlying_buffer) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    vector(std::initializer_list<T> initlist) : base_t(initlist) {}
#endif
};

}


namespace layer3 {

template <class T>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_runtimesize_allocator<T> >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_runtimesize_allocator<T> > base_t;
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
};

}

}
