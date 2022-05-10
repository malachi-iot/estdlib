#ifndef UTIL_EMBEDDED_ISTREAM_H
#define UTIL_EMBEDDED_ISTREAM_H

#include "streambuf.h"
#include "ios.h"
//#include "features.h"
#include "algorithm.h"
#include "traits/char_traits.h"

#include "internal/istream.h"
#include "internal/istream_runtimearray.hpp"
#include "port/istream.h"

//#include <cassert>

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif


// FIX: Re-include this if
//  a) we're sure Arduino and other off-the-beaten-std path has it and
//  b) we feel like fighting with the standard std namespace (which algorithm seems to auto include
//     i.e. it seems to specify 'using namespace std'
//#include <algorithm> // for min function

#ifndef FEATURE_ESTD_IOS_GCOUNT
#define FEATURE_ESTD_IOS_GCOUNT 1
#endif

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


#ifdef ESTD_POSIX
#ifdef __cpp_alias_templates
template<class TChar, class Traits = std::char_traits<TChar> >
using posix_istream = internal::basic_istream< posix_streambuf<TChar, Traits> >;

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
inline internal::basic_istream<TStreambuf, TBase>& ws(
    internal::basic_istream<TStreambuf, TBase>& __is)
{
    typedef typename internal::basic_istream<TStreambuf>::locale_type locale_type;

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

// Experimental because:
// - naming I'm 90% on, not 100%
// - spans prefer to be uint8_t, streams prefer char
namespace experimental {
typedef estd::internal::streambuf<estd::internal::impl::in_span_streambuf<char> > ispanbuf;

typedef estd::internal::basic_istream<ispanbuf> ispanstream;
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
