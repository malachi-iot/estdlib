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
 * Compiles but not runtime tested
 */
template <class TStreambuf, class TBase>
inline detail::basic_istream<TStreambuf, TBase>& ws(
    detail::basic_istream<TStreambuf, TBase>& __is)
{
    typedef typename detail::basic_istream<TStreambuf>::locale_type locale_type;

    locale_type loc = __is.getloc();

    // isspace will automatically fall out if it's an EOF (or nodata)
    for(;;)
    {
        int ch = __is.peek();
        if(isspace((char)ch, loc))
        {
            __is.ignore();
        }
        else
            return __is;
    }
}


// NOTE: Works well, just needs more testing (and hopefully elevation of experimental::num_get
// to non-experimental) before elevating to API level
template <class Streambuf, class Base, class T>
typename enable_if<
    is_arithmetic<T>::value && is_same<T, char>::value == false,
    detail::basic_istream<Streambuf, Base>&>::type
operator >>(
    detail::basic_istream<Streambuf, Base>& in,
    T& value)
{
    // NOTE:
    // "Except where stated otherwise, calling this function does not alter the
    //  value returned by member gcount." [2]
    // Since gcount is mentioned nowhere else on the page, we don't update gcount.
    // A very specific gcount update is mentioned for scenario #11 in [1], which
    // is outside the scope of this method.
    typedef detail::basic_istream<Streambuf, Base> istream_type;
    typedef typename istream_type::streambuf_type streambuf_type;
    typedef typename istream_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;
    typedef estd::istreambuf_iterator<streambuf_type> iterator_type;

    in >> ws;

    iterator_type it(in.rdbuf()), end;
    ios_base::iostate err = in.rdstate();

    num_get<char_type, iterator_type> n;

    n.get(it, end, in, err, value);

    // DEBT: Think I'd prefer a friend operation and pass in 'in' directly on n.get
    // DEBT: It's unclear whether we overwrite or OR state here, but I am betting it's OR and that's
    // what code currently does.  When we find conclusive answer, document it
    in.setstate(err);

    return in;
}

// DEBT: Mate this instead to inherent char_type.  Also, spec calls for unsigned char
// https://en.cppreference.com/w/cpp/io/basic_istream/operator_gtgt2
template <class Streambuf, class Base>
detail::basic_istream<Streambuf, Base>& operator >>(
    detail::basic_istream<Streambuf, Base>& in,
    char& value)
{
    using stream_type = detail::basic_istream<Streambuf, Base>;
    using traits = typename stream_type::traits_type;

    in >> ws;

    const typename traits::int_type c = in.get();

    if(traits::not_eof(c))  value = traits::to_char_type(c);

    return in;
}



// Experimental because:
// - naming I'm 90% on, not 100%
// - spans prefer to be uint8_t, streams prefer char
namespace experimental {
typedef estd::internal::streambuf<estd::internal::impl::in_span_streambuf<char> > ispanbuf;

typedef estd::detail::basic_istream<ispanbuf> ispanstream;
}

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
