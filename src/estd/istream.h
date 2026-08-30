/***
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/io/basic_istream/operator_gtgt
 * 2. https://www.cplusplus.com/reference/istream/istream/operator%3E%3E/
 */
#ifndef UTIL_EMBEDDED_ISTREAM_H
#define UTIL_EMBEDDED_ISTREAM_H

#include "streambuf.h"
#include "ios.h"
//#include "features.h"
#include "algorithm.h"
#include "traits/char_traits.h"

#include "port/istream.h"
#include "internal/istream.h"
#include "internal/istream_runtimearray.hpp"
#include "internal/istream/operators.hpp"

#include "iterator.h"
#include "locale.h"

//#include <cassert>

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif


// FIX: Re-include this if
//  a) we're sure Arduino and other off-the-beaten-std path has it and
//  b) we feel like fighting with the standard std namespace (which algorithm seems to auto include
//     i.e. it seems to specify 'using namespace std'
//#include <algorithm> // for min function

namespace estd {

/*
#ifdef min
#undef min
#endif

// lifted from http://www.cplusplus.com/reference/algorithm/min/
// FIX: put this into algorithm.h if we're gonna really roll with it
template <class T> const T& min (const T& a, const T& b)
{
    return !(b<a)?a:b;     // or: return !comp(b,a)?a:b; for version (2)
}*/


#ifdef FEATURE_POSIX_IOS
#ifdef __cpp_alias_templates
template<class Char, class Traits = std::char_traits<Char> >
using posix_istream = detail::basic_istream< posix_streambuf<Char, Traits> >;

typedef posix_istream<char> istream;
#endif
#endif


// To change delimiters, we'll need to do something like this:
// http://stackoverflow.com/questions/7302996/changing-the-delimiter-for-cin-c
/*
inline basic_istream<char>& operator >>(basic_istream<char>& in, short& value)
{
    return *in;
}
*/

// NOTE: Due to C++ rules disallowing partial function specialization, we can't do
// a prototype here.  I would prefer specialization
// so that we can link in specialized implementations instead of forcing inline-includes
// but inline-includes are reasonable
// discussed here https://www.fluentcpp.com/2017/08/15/function-templates-partial-specialization-cpp/
//template <class TImpl, class TBase, class T>
//internal::basic_istream<TImpl, TBase>& operator >>(internal::basic_istream<TImpl, TBase>& in,
//                                                   T& value);


/**
 * Consume whitespace
 * @tparam TChar
 * @param __os
 * @return
 *
 */
template <class Streambuf, class Base>
inline detail::basic_istream<Streambuf, Base>& ws(
    detail::basic_istream<Streambuf, Base>& __is)
{
    using locale_type = typename detail::basic_istream<Streambuf>::locale_type;
    using streambuf_type = remove_reference_t<Streambuf>;
    using traits = typename streambuf_type::traits_type;
    using ct = estd::ctype<estd::remove_const_t<typename traits::char_type>, locale_type>;

    //locale_type loc = __is.getloc();

    // isspace will automatically fall out if it's an EOF (or nodata)
    for(;;)
    {
        const typename traits::int_type ch = __is.peek();
        // This works and is the 'std' way, but ours custom way is a little quicker
        //if(isspace((char_type)ch, loc))
        if(ct::isspace(traits::to_char_type(ch)))
        {
            __is.ignore();
        }
        else
            return __is;
    }
}



// 05NOV24 MB - perhaps these would more comfortably
// live in estd itself rather than estd::detail
// 19JUL26 MB - align with relatively new https://en.cppreference.com/cpp/io/basic_spanbuf
namespace detail {

// We prefer our own detail::char_traits regardless of FEATURE_ESTD_CHARTRAITS because ispanbuf in
// particular wants const specialization (see https://github.com/malachi-iot/estdlib/issues/220)

template <class Char, class CharTraits = estd::detail::char_traits<Char>, size_t Extent = detail::dynamic_extent::value>
using basic_ispanbuf = estd::internal::streambuf<estd::internal::impl::in_span_streambuf<CharTraits, Extent>>;

template <class Char, class CharTraits = estd::detail::char_traits<Char>, size_t Extent = detail::dynamic_extent::value>
using basic_ispanstream = basic_istream<basic_ispanbuf<Char, CharTraits, Extent>>;

// EXPERIMENTAL
// Seems unhappy since it's a perfect forward Args&& situation and can't directly match
// 'span' as a parameter
#if __cpp_deduction_guides && !__clang__
template <class Char, size_t Extent>
basic_istream(span<Char, Extent>) -> basic_ispanstream<Char, estd::char_traits<Char>, Extent>;
#endif

}

using ispanbuf = detail::basic_ispanbuf<char>;
using ispanstream = detail::basic_ispanstream<char>;

// Working out best way for consumers to really configure their istreams
namespace experimental {
#ifdef __cpp_alias_templates

/*
template <bool v>
using Range = estd::internal::Range<v>;

//template<class TStreambuf>
//using flagged_istream = estd::internal::basic_istream< TStreambuf >;
template <class TStreambuf, istream_flags::flag_type flags = istream_flags::_default, typename = Range<true> >
class flagged_istream;

// Concept seems to work, though definitely fiddly.  Try:
// https://softwareengineering.stackexchange.com/questions/194412/using-scoped-enums-for-bit-flags-in-c
template <class TStreambuf, istream_flags::flag_type flags>
class flagged_istream<TStreambuf, flags, 
    Range<(flags & istream_flags::block_mask) == istream_flags::non_blocking> > : 
    public estd::internal::basic_istream
        <TStreambuf, estd::internal::basic_ios
            <TStreambuf, false, estd::internal::ios_base_policy<TStreambuf> > >
{

};

template <class TStreambuf>
class flagged_istream<TStreambuf, istream_flags::blocking> : public estd::internal::basic_istream
    <TStreambuf, estd::internal::basic_ios
        <TStreambuf, false, estd::internal::ios_base_policy<TStreambuf> > >
{

}; */


#endif

}


}
#endif //UTIL_EMBEDDED_ISTREAM_H
