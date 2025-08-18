#ifndef UTIL_EMBEDDED_OSTREAM_H
#define UTIL_EMBEDDED_OSTREAM_H

#include "iosfwd.h"

#if __cplusplus >= 201103L
// NOTE: Requirements manual enable because it's a fragile feature still
// (steady_clock isn't automatically available everywhere)
// permits ostream timeout capabilities, but you still need to enable it with policy
// as well (though it's default to on right now).  #ifdef'ing because chrono is currently
// highly c++11 dependent
//#define FEATURE_ESTD_OSTREAM_TIMEOUT
#else
#include "c++03/ostream.h"
#endif

#include "streambuf.h"
#include "ios.h"
#include "charconv.h"
#include "limits.h"
#include "internal/string_convert.h"
#include "internal/iomanip.h"
#include "traits/char_traits.h"
#include "internal/ostream.h"
#ifdef FEATURE_ESTD_OSTREAM_TIMEOUT
#include "chrono.h"
#include "thread.h"
#endif
#include "internal/ostream_basic_string.hpp"
#include "port/ostream.h"

#if FEATURE_STD
#include <charconv>
#elif __AVR__
// https://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html
// for dtostrf
#include <stdlib.h>
#endif

namespace estd {

namespace detail {

// Using TBase::char_type as it's the most reliable non-reference
template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator <<(basic_ostream<TStreambuf, TBase>& out,
                                                         const typename TBase::char_type* s)
{
    typedef typename TBase::traits_type traits_type;
    return out.write(s, traits_type::length(s));
}

#if __cplusplus >= 201103L
template <class TStreambuf, class TBase, typename T,
        class enabled = enable_if_t<(estd::numeric_limits<T>::is_integer)> >
basic_ostream<TStreambuf, TBase>&
#else
template <class TStreambuf, class TBase, typename T>
typename estd::enable_if<estd::numeric_limits<T>::is_integer, basic_ostream<TStreambuf, TBase>&>::type
#endif
    operator<<(basic_ostream<TStreambuf, TBase>& out, T value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase, typename T>
typename enable_if<is_floating_point<T>::value, basic_ostream<TStreambuf, TBase>&>::type
operator <<(basic_ostream<TStreambuf, TBase>& out, T v)
{
    // DEBT: Crude floating point conversions.  Really we need fully fledged
    // num_put as per https://github.com/malachi-iot/estdlib/issues/23
    // DEBT: In the meantime, we can probably heed more width/precision flags
    // supplied by ostream itself for both formatting as well as overrun protection
    char temp[32];
#if FEATURE_STD_CHARCONV
    const std::chars_format format{std::chars_format::fixed};
    const std::to_chars_result r = std::to_chars(temp, temp + 32, v, format,
        out.precision());

    out.write(temp, r.ptr - temp);
#elif __AVR__
    out << dtostrf(v, out.width(), out.precision(), temp);
#else
    static_assert(!is_floating_point<T>::value, "Not yet supported");
#endif
    return out;
}


template <class Streambuf, class Base>
inline basic_ostream<Streambuf, Base>& operator <<(basic_ostream<Streambuf, Base>& out,
    typename Base::char_type ch)
{
#if FEATURE_ESTD_OSTREAM_SETW
    const streamsize pad = out.width();

    out.fill_n(pad - 1);
    out.width(0);
#endif

    return out.put(ch);
}


template <class Streambuf, class Base>
inline basic_ostream<Streambuf, Base>& operator <<(basic_ostream<Streambuf, Base>& out,
    bool v)
{
    // DEBT: Very crude.  Needs bounds check, width/padding, and maybe locale
    layer1::string<16> s;
    num_put<char, char*> n;

    *n.put(s.data(), out, ' ', v) = 0;

    return out << s;
}


// Somehow, blackfin has the PRIXPTR and friends even though it doesn't have
// inttypes.h or cinttypes.  
#if defined(FEATURE_STD_INTTYPES) || defined(__ADSPBLACKFIN__)
// explicit prototype here to avoid pulling in collision-happy stdio
// watch out for linker errors if your environment doesn't have this call
int snprintf( char* buffer, std::size_t buf_size, const char* format, ... );
#endif

template <class TStreambuf>
inline basic_ostream<TStreambuf>& operator<<(basic_ostream<TStreambuf>& out, void* addr)
{
    char buffer[sizeof(uintptr_t) * 3];

#ifdef ESP_OPEN_RTOS
    __utoa((uint32_t)addr, buffer, 16);
#elif defined(FEATURE_STD_INTTYPES) || defined(__ADSPBLACKFIN__)
    // DEBT: Really don't like relying on snprintf and seems like rendering a pointer is
    // really just a hex int -> string conversion
    snprintf(buffer, sizeof(buffer), "%" PRIXPTR, (uintptr_t)addr);
#else
#error Not implemented
#endif
    return out << buffer;
}

#if __cpp_rvalue_references
// DEBT: Would really like to filter this out by is_invocable, but we don't have that
// in estd yet
template <class Streambuf, class Base, class F,
    enable_if_t<is_base_of<ostream_functor_tag, F>::value, bool> = true>
constexpr basic_ostream<Streambuf, Base>& operator <<(basic_ostream<Streambuf, Base>& out, F&& f)
{
    return (f(out), out);
}
#endif


}

#ifdef __cpp_alias_templates
template <class TChar, class CharTraits = estd::char_traits<TChar> >
using basic_ostream = detail::basic_ostream<
    basic_streambuf<TChar, CharTraits>,
    internal::basic_ios<basic_streambuf<TChar, CharTraits>, true> >;

typedef basic_ostream<char> ostream;
#endif

namespace experimental {

// manage both a native TStreambuf as well as a pointer to a traditional-style
// basic_streambuf.  Note that this will auto wrap TStreambuf, because otherwise
// if TStreambuf didn't need wrapping, you wouldn't use wrapped_ostream in the first
// place (you'd instead use traditional basic_ostream)
template <class TStreambuf, class TBase =
        estd::internal::basic_ios<estd::basic_streambuf<
            typename estd::remove_reference<TStreambuf>::type::char_type,
#if FEATURE_ESTD_STREAMBUF_TRAITS
            typename estd::remove_reference<TStreambuf>::type::traits_type::char_traits
#else
            typename estd::remove_reference<TStreambuf>::type::traits_type
#endif
            >, true>
        >
struct wrapped_ostream : estd::detail::basic_ostream<
        estd::basic_streambuf<
            typename TBase::char_type,
#if FEATURE_ESTD_STREAMBUF_TRAITS
            typename TBase::traits_type::char_traits>,
#else
            typename TBase::traits_type>,
#endif
        TBase>
{
    typedef estd::detail::basic_ostream<
        estd::basic_streambuf<
            typename TBase::char_type,
#if FEATURE_ESTD_STREAMBUF_TRAITS
            typename TBase::traits_type::char_traits>,
#else
            typename TBase::traits_type>,
#endif
        TBase> base_type;

    // NOTE: Not well supported TStreambuf being a value vs a reference yet, needs work
    typedef estd::internal::basic_streambuf_wrapped<TStreambuf> streambuf_type;

    // need a value type here so that the wrapped streambuf has a place to live
    // i.e. be allocated
    streambuf_type wrapped_streambuf;

    wrapped_ostream(TStreambuf& native_streambuf) :
        base_type(&wrapped_streambuf),
        wrapped_streambuf(native_streambuf)
    {

    }
};


template <class TStreambuf, class TBase>
wrapped_ostream<TStreambuf&>
convert(estd::detail::basic_ostream<TStreambuf, TBase>& os)
{
    wrapped_ostream<TStreambuf&> wrapped_os(*os.rdbuf());
    return wrapped_os;
}

}





// TODO: Put this in layer1/layer2 since it isn't the traditional/fully virtual-capable version
#ifdef FEATURE_POSIX_IOS
#ifdef __cpp_alias_templates
template<class TChar, class Traits = std::char_traits<TChar> >
using posix_ostream = detail::basic_ostream< posix_streambuf<TChar, Traits> >;

//typedef posix_ostream<char> ostream;
#endif
#endif

namespace detail {

template <class Char, class Traits = estd::char_traits<Char>>
using basic_ospanbuf = estd::detail::streambuf<
    estd::internal::impl::out_span_streambuf<Traits>>;

// DEPRECATED
using ospanbuf = basic_ospanbuf<char>;

template <class Char, class Traits = estd::char_traits<Char>>
using basic_ospanstream = basic_ostream<basic_ospanbuf<Char, Traits>>;

// DEPRECATED
using ospanstream = basic_ospanstream<char>;

}

using ospanbuf = detail::basic_ospanbuf<char>;
using ospanstream = detail::basic_ospanstream<char>;


}

#endif //UTIL_EMBEDDED_OSTREAM_H
