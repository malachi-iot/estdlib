#pragma once

#include "ostream.h"
#include "istream.h"
#include "string.h"
#include "internal/impl/streambuf/string.h"

// TODO: Move non-alias template flavor elsewhere, since it's so bulky
namespace estd {

namespace layer1 {
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_stringbuf = estd::internal::streambuf <
    estd::internal::impl::basic_stringbuf<layer1::basic_string < TChar, N, null_terminated, Traits> > >;

template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_out_stringbuf = estd::internal::streambuf <
    estd::internal::impl::out_stringbuf<layer1::basic_string < TChar, N, null_terminated, Traits> > >;

template<size_t N, bool null_terminated = true>
using stringbuf = basic_stringbuf<char, N, null_terminated>;
#else
template <class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
struct basic_stringbuf : estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<basic_string<TChar, N, null_terminated, Traits> >
        >
{
    typedef estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<basic_string<TChar, N, null_terminated, Traits> >
        > base_type;
    ESTD_CPP_FORWARDING_CTOR(basic_stringbuf)
};
#endif
}

namespace experimental {
// all these were (are) layer1, but not sure we want this kind of fused
// data + formatting functionality
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_ostringstream = estd::internal::basic_ostream<layer1::basic_out_stringbuf<TChar, N, null_terminated, Traits> >;

template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_istringstream = estd::internal::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> >;

template<size_t N, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;
#else
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
struct basic_istringstream : estd::detail::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> >
{
    typedef estd::detail::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> > base_type;
    ESTD_CPP_FORWARDING_CTOR(basic_istringstream)
};

template<size_t N, bool null_terminated = true>
struct istringstream : basic_istringstream<char, N, null_terminated>
{
    typedef basic_istringstream<char, N, null_terminated> base_type;
    ESTD_CPP_FORWARDING_CTOR(istringstream)
};

template <size_t N, bool null_terminated = true>
struct ostringstream : estd::detail::basic_ostream<layer1::basic_stringbuf<char, N, null_terminated> >
{
    typedef estd::detail::basic_ostream<layer1::basic_stringbuf<char, N, null_terminated> > base_type;
    ESTD_CPP_FORWARDING_CTOR(ostringstream)
};
#endif

}

namespace layer2 {

#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = estd::char_traits<TChar> >
using basic_stringbuf = internal::streambuf <
    internal::impl::basic_stringbuf<
        layer2::basic_string < TChar, N, null_terminated, Traits> > >;

typedef basic_stringbuf<char, 0> stringbuf;
#endif

}


namespace layer3 {

// NOTE: not defaulting null_terminated yet as I'm not sure if we want to lean one way or the
// other for layer3 ostringstream
// also - code compiles but is untested
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, bool null_terminated, class Traits = estd::char_traits<TChar> >
using basic_stringbuf = estd::internal::streambuf <
    estd::internal::impl::basic_stringbuf<
        layer3::basic_string < TChar, null_terminated, Traits> > >;

typedef basic_stringbuf<char, 0> stringbuf;

template<class TChar, bool null_terminated, class Traits = estd::char_traits<TChar> >
using basic_ostringstream = estd::internal::basic_ostream<basic_stringbuf<TChar, null_terminated, Traits> >;

#endif
}

}
