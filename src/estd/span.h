#pragma once

#include "internal/buffer.h"

namespace estd {

#ifdef FEATURE_CPP_INLINE_VARIABLES
inline CONSTEXPR ptrdiff_t dynamic_extent = -1;
#else
// hate doing global collision-prone defines like this
//#define dynamic_extent = -1
#endif

template <class T, ptrdiff_t Extent = -1,
          class TBase = typename estd::conditional<Extent == -1,
                      estd::internal::layer3::buffer<T, size_t>,
                      // NOTE: Need explicit size_t trailing here because otherwise -1 sneaks
                      // its way in
                      estd::layer2::array<T, static_cast<size_t>(Extent), size_t> >::type>
class span : public TBase
{
    typedef TBase base_t;

    static CONSTEXPR bool is_dynamic = Extent == -1;

public:
    static CONSTEXPR ptrdiff_t extent = Extent;

    typedef T element_type;
    typedef T* pointer;
    typedef typename base_t::size_type index_type;
    typedef typename estd::remove_cv<T>::type value_type;

    span(pointer data, index_type size) :
            base_t(data, size) {}

    // ExtendLocal needed because SFINAE function selection needs that
    // fluidity
    // dynamic flavor
#ifdef FEATURE_CPP_DEFAULT_TARGS
    template <size_t N, ptrdiff_t ExtentLocal = Extent,
              class ExtentOnly = typename enable_if<ExtentLocal == -1>::type>
    span(element_type (&data) [N]) : base_t(data, N) {}

    // constant size flavor
    template <size_t N, ptrdiff_t ExtentLocal = Extent,
              class ExtentOnly = typename enable_if<ExtentLocal == N>::type>
    span(element_type (&data) [N], bool = true) : base_t(data) {}
#endif

    // most definitely a 'shallow clone'
    span(const base_t& clone_from) : base_t(clone_from) {}
};



// this one in particular is getting some use and performing well
// TODO: Move this out into span
typedef span<const uint8_t> const_buffer;
typedef span<uint8_t> mutable_buffer;


}
