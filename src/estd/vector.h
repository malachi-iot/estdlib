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
    typedef typename base_t::size_type size_type;
    typedef typename base_t::accessor accessor;
    typedef typename base_t::iterator iterator;

    allocator_type& get_allocator()
    {
        return base_t::get_allocator();
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    // NOTE: because pos requires a non-const lock, we can't do traditional
    // const_iterator here
    iterator insert(iterator pos, const_reference value)
    {
        T* a = base_t::lock();

        T& pos_item = pos.lock();

        // all very raw array dependent
        base_t::raw_insert(a, &pos_item, &value);

        pos.unlock();

        base_t::unlock();

        return pos;
    }
#else
    iterator insert(iterator pos, const T& value)
    {

    }
#endif
};


namespace layer1 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N > >
{
};

};


namespace layer2 {

template <class T, size_t N>
class vector : public estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > >
{
    typedef estd::vector<T, estd::internal::single_fixedbuf_allocator<T, N, false, T* > > base_t;

public:
    vector(T* underlying_buffer) : base_t(underlying_buffer) {}
};

}

}
