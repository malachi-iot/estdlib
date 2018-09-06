#pragma once

#include "internal/buffer.h"

namespace estd {

#ifdef FEATURE_CPP_INLINE_VARIABLES
inline CONSTEXPR ptrdiff_t dynamic_extent = -1;
#endif

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class T, ptrdiff_t Extent = -1>
using span = estd::internal::layer3::mutable_buffer<T, size_t>;
#else
template <class T, class TSize = size_t>
class span : public estd::internal::layer3::mutable_buffer<T, size_t>
{
    typedef estd::internal::layer3::mutable_buffer<T, size_t> base_t;

public:
    typedef typename base_t::size_type size_type;
    typedef typename base_t::value_type value_type;

    span(value_type* data, size_type size) :
            base_t(data, size) {}

    template <size_t N>
    span(value_type (&data) [N]) : base_t(data, N) {}

    // most definitely a 'shallow clone'
    span(const base_t& clone_from) : base_t(clone_from) {}
};
#endif

// this one in particular is getting some use and performing well
// TODO: Move this out into span
typedef span<const uint8_t> const_buffer;
typedef span<uint8_t> mutable_buffer;


}
