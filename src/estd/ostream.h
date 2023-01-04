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
#include "traits/char_traits.h"
#include "internal/ostream.h"
#ifdef FEATURE_ESTD_OSTREAM_TIMEOUT
#include "chrono.h"
#include "thread.h"
#endif
#include "internal/ostream_basic_string.hpp"
#include "port/ostream.h"

namespace estd {

namespace internal {

// Using TBase::char_type as it's the most reliable non-reference
template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator <<(basic_ostream<TStreambuf, TBase>& out,
                                                         const typename TBase::char_type* s)
{
    typedef typename TBase::traits_type traits_type;
    return out.write(s, traits_type::length(s));
}

// helper since we often convert a statically allocated string
// NOTE: this will behave slightly differently than a regular string, see to_chars_opt
// DEBT: Move this to charconv
template <unsigned N, typename TInt>
inline to_chars_result to_string_opt(char (&buffer)[N], TInt value, unsigned base)
{
    // -1 here because to_chars doesn't care about null termination, but we do
    to_chars_result result = to_chars_opt(buffer, buffer + N - 2, value, base);

    // DEBT: Check result for conversion failure

    // remember, opt flavor specifies 'ptr' as beginning and we must manually
    // null terminate the end (ala standard to_chars operation)
    buffer[N - 1] = 0;

    return result;
}

// Internal call - write an integer of the specified base to the output stream
// DEBT: No locale num_put available yet.
// to_string_opt is less overhead so really we'd like to compile time choose
// one or the other
template <unsigned base, class TStreambuf, class TBase, class T>
inline basic_ostream<TStreambuf, TBase>& write_int(basic_ostream<TStreambuf, TBase>& out, T value)
{
    // +1 for potential - sign
    // +1 for null terminator
    char buffer[estd::numeric_limits<T>::template length<base>::value + 2];

    to_chars_result result = to_string_opt(buffer, value, base);

    int sz = &buffer[sizeof(buffer) - 1] - result.ptr;

    return out.write(result.ptr, sz);
}

template <class TStreambuf, class TBase, typename TInt>
basic_ostream<TStreambuf, TBase>& out_int_helper(basic_ostream<TStreambuf, TBase>& out, TInt value)
{
    // DEBT: another typical enum -> traits/template conversion - a framework
    // support for that really would be useful
    switch(out.flags() & ios_base::basefield)
    {
        case ios_base::oct:
            return write_int<8>(out, value);

        case ios_base::dec:
            return write_int<10>(out, value);

        case ios_base::hex:
            return write_int<16>(out, value);

        default:
            // TODO: assert or log an error condition
            return out;
    }
}


#if __cplusplus >= 201103L
template <class TStreambuf, class TBase, typename T,
        class enabled = enable_if_t<(estd::numeric_limits<T>::is_integer)> >
basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf>& out, T value)
{
    return out_int_helper(out, value);
}
#else
// DEBT: Move all this out to c++03 specific area
template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, int value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, unsigned value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, long value)
{
    return out_int_helper(out, value);
}

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator<<(basic_ostream<TStreambuf, TBase>& out, unsigned long value)
{
    return out_int_helper(out, value);
}

#endif

template <class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator <<(basic_ostream<TStreambuf, TBase>& out,
                                                        typename TBase::char_type ch)
{
    return out.put(ch);
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
    snprintf(buffer, sizeof(buffer), "%" PRIXPTR, (uintptr_t)addr);
#else
#error Not implemented
#endif
    return out << buffer;
}


}

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class TChar, class CharTraits = estd::char_traits<TChar> >
using basic_ostream = internal::basic_ostream<
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
            typename estd::remove_reference<TStreambuf>::type::traits_type
            >, true>
        >
struct wrapped_ostream : estd::internal::basic_ostream<
        estd::basic_streambuf<
            typename TBase::char_type,
            typename TBase::traits_type>,
        TBase>
{
    typedef estd::internal::basic_ostream<
        estd::basic_streambuf<
            typename TBase::char_type,
            typename TBase::traits_type>,
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
convert(estd::internal::basic_ostream<TStreambuf, TBase>& os)
{
    wrapped_ostream<TStreambuf&> wrapped_os(*os.rdbuf());
    return wrapped_os;
}

}


template <class TStreambuf, class TBase>
inline internal::basic_ostream<TStreambuf, TBase>& endl(
    internal::basic_ostream<TStreambuf, TBase>& os)
{
    // uses specialized call to bypass sentry so that we don't needlessly check
    // unitbuf and potentially double-flush
    os.put(os.widen('\n'), true);
    os.flush();
    return os;
}

// We have this out here rather than ios_base because of our deviation from virtual methods
template <class TStreambuf>
inline internal::basic_ostream<TStreambuf>& dec(internal::basic_ostream<TStreambuf>& __os)
{
    __os.setf(ios_base::dec, ios_base::basefield);
    return __os;
}

template <class TStreambuf>
inline internal::basic_ostream<TStreambuf>& hex(internal::basic_ostream<TStreambuf>& __os)
{
    __os.setf(ios_base::hex, ios_base::basefield);
    return __os;
}


// TODO: Put this in layer1/layer2 since it isn't the traditional/fully virtual-capable version
#ifdef FEATURE_POSIX_IOS
#ifdef __cpp_alias_templates
template<class TChar, class Traits = std::char_traits<TChar> >
using posix_ostream = internal::basic_ostream< posix_streambuf<TChar, Traits> >;

//typedef posix_ostream<char> ostream;
#endif
#endif

// Experimental because:
// - naming I'm 90% on, not 100%
// - spans prefer to be uint8_t, streams prefer char
namespace experimental {
typedef estd::internal::streambuf<estd::internal::impl::out_span_streambuf<char> > ospanbuf;

typedef estd::internal::basic_ostream<ospanbuf> ospanstream;
}

}

#endif //UTIL_EMBEDDED_OSTREAM_H
