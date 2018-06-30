#pragma once

#include "array.h"

namespace estd {

// TODO: once we lean array itself on allocated_array, beef up constructors here
template <class T, class TSize = size_t>
class span : public estd::layer3::array<T, size_t>
{
    typedef estd::layer3::array<T, size_t> base_t;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::value_type value_type;

public:
    // This is a low level call, but buffers are low level creatures
    // gently discouraged during mutable_buffer,
    // strongly discouraged during const_buffer,
    // but not necessarily wrong to use it
    void resize(size_type n) { base_t::m_size = n; }

    span(value_type* data, size_type size) :
            base_t(data, size) {}

    template <size_t N>
    span(value_type (&data) [N]) : base_t(data, N) {}

    // most definitely a 'shallow clone'
    span(const span& clone_from) :
            base_t(clone_from.data(), clone_from.size()) {}
};

}