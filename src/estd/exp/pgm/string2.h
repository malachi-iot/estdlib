#pragma once

#include "../pgm_string.h"

#include "../../internal/macro/push.h"

namespace estd {

namespace internal {

template <typename T, unsigned N>
struct dynamic_array_helper<experimental::pgm_array_string<T, N> >
{
    typedef experimental::pgm_array_string<T, N> impl_type;
    typedef internal::dynamic_array<impl_type> dynamic_array;
    typedef internal::allocated_array<impl_type> array;

    typedef typename array::value_type value_type;
    typedef typename array::pointer pointer;
    typedef typename array::const_pointer const_pointer;
    typedef typename array::size_type size_type;

    // copy from us to outside dest/other
    static size_type copy_to(const array& a,
        typename estd::remove_const<value_type>::type* dest,
        size_type count, size_type pos = 0)
    {
        const size_type _end = estd::min(count, a.size());
        memcpy_P(dest, a.offset(pos), _end * sizeof(T));
        return _end;
    }
};

// DEBT: We have to manually specialize this guy too because we DON'T
// expose locking_preference at allocator_traits level
template <class O, unsigned N>
struct out_string_helper<O, experimental::pgm_array_string<char, N> >
{
    template <class A>
    static void out(O& out, const A& str)
    {
        out_string_helper_iterated(out, str);
    }
};


}   // estd::internal


template <size_t N = internal::variant_npos()>
struct basic_pgm_string2 :
    internal::basic_string2<experimental::pgm_array_string<char, N> >
{
    using base_type = internal::basic_string2<
        experimental::pgm_array_string<char, N> >;

    constexpr basic_pgm_string2(const char* const s) :
        base_type(s)
    {}
};


}

#include "../../internal/macro/pop.h"
