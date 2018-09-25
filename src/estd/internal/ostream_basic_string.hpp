#pragma once

#include "../string.h"

namespace estd {
namespace internal {

template <class TStreambuf, class TBase>
struct basic_ostream;

}
template <class TStreambuf, class TBase, class TStringAllocator, class TStringPolicy>
inline internal::basic_ostream<TStreambuf, TBase>& operator <<(
        internal::basic_ostream<TStreambuf, TBase>& out,
        const estd::basic_string<
            typename TStreambuf::char_type,
            typename TStreambuf::traits_type,
            TStringAllocator,
            TStringPolicy >& str
        )
{
    out.write(str.clock(), str.size());

    str.cunlock();

    return out;
}

// catch-all case, mainly useful for const_string varieties
// since const char and char don't align between ostream
// and string.  However, useful for other dynamic_array
// derived classes... likely including regular strings too
template <class TStreambuf, class TBase, class TStringImpl>
inline internal::basic_ostream<TStreambuf, TBase>& operator <<(
        internal::basic_ostream<TStreambuf, TBase>& out,
        const internal::dynamic_array<TStringImpl>& str)
{
#ifdef FEATURE_CPP_STATIC_ASSERT
    typedef typename TStreambuf::char_type l_char_type;

    typedef typename internal::dynamic_array<TStringImpl>::value_type _r_char_type;
    typedef typename estd::remove_const<_r_char_type>::type r_char_type;

    static_assert (estd::is_same<l_char_type, r_char_type>::value, "character types must match");
#endif

    out.write(str.clock(), str.size());
    str.cunlock();
    return out;
}

}
