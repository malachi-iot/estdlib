#pragma once

#include "ostream.h"
#include "istream.h"
#include "string.h"

namespace estd {

namespace layer1 {
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
using basic_stringbuf = internal::streambuf <
    internal::impl::basic_stringbuf<layer1::basic_string < TChar, N, null_terminated, Traits> > >;

template<class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
using basic_out_stringbuf = internal::streambuf <
    internal::impl::out_stringbuf<layer1::basic_string < TChar, N, null_terminated, Traits> > >;

template<size_t N, bool null_terminated = true>
using stringbuf = basic_stringbuf<char, N, null_terminated>;
#else
template <class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
struct basic_stringbuf : internal::streambuf<
        internal::impl::basic_stringbuf<basic_string<TChar, N, null_terminated, Traits> >
        >
{

};
#endif
}

namespace experimental {
// all these were (are) layer1, but not sure we want this kind of fused
// data + formatting functionality
#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
using basic_ostringstream = internal::basic_ostream<layer1::basic_out_stringbuf<TChar, N, null_terminated, Traits> >;

template<class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
using basic_istringstream = internal::basic_istream<layer1::basic_stringbuf<TChar, N, null_terminated, Traits> >;

template<size_t N, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;
#endif

}

namespace layer2 {

#ifdef FEATURE_CPP_ALIASTEMPLATE
template<class TChar, size_t N, bool null_terminated = true, class Traits = std::char_traits<TChar> >
using basic_stringbuf = internal::streambuf <
    internal::impl::basic_stringbuf<
        layer2::basic_string < TChar, N, null_terminated, Traits> > >;

typedef basic_stringbuf<char, 0> stringbuf;
#endif

}

}