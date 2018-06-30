#pragma once

#include "internal/buffer.h"

namespace estd {

// this one in particular is getting some use and performing well
// TODO: Move this out into span
typedef internal::layer3::buffer<const uint8_t> const_buffer;
typedef internal::layer3::buffer<uint8_t> mutable_buffer;

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class T>
using span = estd::internal::layer3::buffer<T, size_t>;
#else
template <class T, class TSize = size_t>
class span : public estd::internal::layer3::buffer<T, size_t>
{
    typedef estd::internal::layer3::buffer<T, size_t> base_t;
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
            base_t(clone_from) {}
};
#endif


}
