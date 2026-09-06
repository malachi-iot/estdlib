#pragma once

#include "iostream.h"
#include "string.h"
#include "internal/impl/streambuf/string.h"
#include "internal/impl/streambuf/string_view.h"

namespace estd {

namespace layer1 {

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringbuf = estd::internal::streambuf <
    estd::internal::impl::basic_stringbuf<layer1::basic_string < Char, N, null_terminated, Traits> > >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_out_stringbuf = estd::internal::streambuf <
    estd::internal::impl::out_stringbuf<layer1::basic_string < Char, N, null_terminated, Traits> > >;

template<size_t N, bool null_terminated = true>
using stringbuf = basic_stringbuf<char, N, null_terminated>;

}


namespace layer1 {
template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_ostringstream = estd::detail::basic_ostream<layer1::basic_out_stringbuf<Char, N, null_terminated, Traits> >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_istringstream = estd::detail::basic_istream<layer1::basic_stringbuf<Char, N, null_terminated, Traits> >;

template<class Char, size_t N, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringstream = estd::detail::basic_iostream<layer1::basic_stringbuf<Char, N, null_terminated, Traits> >;

template<size_t N, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;

template<size_t N, bool null_terminated = true>
using stringstream = basic_stringstream<char, N, null_terminated>;

}


namespace layer2 {

template<class Char, size_t N = 0, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringbuf = detail::streambuf <
    internal::impl::basic_stringbuf<
        layer2::basic_string <Char, N, null_terminated, Traits> > >;

using stringbuf = basic_stringbuf<char>;

template <class Char, size_t N = 0, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_istringstream =
    detail::basic_istream<basic_stringbuf<Char, N, null_terminated, Traits>>;

// Lightly tested
template <class Char, size_t N = 0, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_ostringstream =
    detail::basic_ostream<basic_stringbuf<Char, N, null_terminated, Traits>>;

// Lightly tested
template<size_t N = 0, bool null_terminated = true>
using istringstream = basic_istringstream<char, N, null_terminated>;

// Lightly tested
template<size_t N = 0, bool null_terminated = true>
using ostringstream = basic_ostringstream<char, N, null_terminated>;

}


namespace layer3 {

// Default to null terminated to align with C/C++ convention (even std::string c_str gives us this)
template<class Char, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_stringbuf = detail::streambuf <
    estd::internal::impl::basic_stringbuf<
        layer3::basic_string <Char, null_terminated, Traits> > >;

using stringbuf = basic_stringbuf<char>;

// Not ready yet
template<class Char, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_istringstream =
    detail::basic_istream<basic_stringbuf<Char, null_terminated, Traits> >;

template<class Char, bool null_terminated = true, class Traits = estd::char_traits<Char> >
using basic_ostringstream =
    detail::basic_ostream<basic_stringbuf<Char, null_terminated, Traits> >;

using ostringstream = basic_ostringstream<char>;

}

}
