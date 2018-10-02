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

    constexpr index_type size_bytes() const
    { return base_t::size() * sizeof(element_type); }

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    span(pointer data, index_type count) :
            base_t(data, count) {}

    // ExtendLocal needed because SFINAE function selection needs that
    // fluidity
    // dynamic flavor
#ifdef FEATURE_CPP_DEFAULT_TARGS
    template <size_t N, ptrdiff_t ExtentLocal = Extent,
              class ExtentOnly = typename enable_if<ExtentLocal == -1>::type>
    constexpr span(element_type (&data) [N]) : base_t(data, N) {}

    // constant size flavor
    template <size_t N, ptrdiff_t ExtentLocal = Extent,
              class ExtentOnly = typename enable_if<ExtentLocal == N>::type>
    constexpr span(element_type (&data) [N], bool = true) : base_t(data) {}
#endif

    // most definitely a 'shallow clone'
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    span(const base_t& clone_from) : base_t(clone_from) {}

    span<T, -1> subspan(ptrdiff_t offset, ptrdiff_t count = -1)
    {
        if(count == -1) count = base_t::size();

        pointer data = base_t::data();

        return span<T, -1>(data + offset, count);
    }
};



typedef span<const uint8_t> const_buffer;
typedef span<uint8_t> mutable_buffer;

// FIX: Doesn't work yet
template <class T, ptrdiff_t N, ptrdiff_t S = N == -1 ? -1 : N * sizeof(T) >
span<const byte, S> as_bytes(span<T, N> s) noexcept
{
    return span<const byte, S>(reinterpret_cast<const byte*>(s.data()), s.size_bytes());
}

}
