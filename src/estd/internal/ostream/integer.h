#pragma once

#include "../ios.h"

namespace estd {

namespace internal {

// Low level helper, hopefully gets un-inlined and saves some code space
template <class Streambuf, class Base>
void write_filled_buffer(
    detail::basic_ostream<Streambuf, Base>& out,
    typename remove_cvref<Streambuf>::type::char_type* buffer,
    unsigned sz)
{
#if FEATURE_ESTD_OSTREAM_SETW
    const streamsize pad = out.width();
    out.width(0);

    if(out.flags() & ios_base::right)   out.fill_n(pad - sz);
#endif

    out.write(buffer, sz);

#if FEATURE_ESTD_OSTREAM_SETW
    if(out.flags() & ios_base::left)    out.fill_n(pad - sz);
#endif
}

#if UNUSED
// Internal call - write an integer of the specified base to the output stream
// DEBT: No locale num_put available yet.
// to_string_opt is less overhead so really we'd like to compile time choose
// one or the other
template <unsigned base, class Streambuf, class Base, class Numeric>
inline detail::basic_ostream<Streambuf, Base>& write_int(detail::basic_ostream<Streambuf, Base>& out, Numeric value)
{
    using char_type = typename remove_cvref<Streambuf>::type::char_type;
    //using num_put = internal::num_put<char_type, char_type*>;

    // +1 for potential - sign
    // +0 for null terminator, none required
    constexpr unsigned N = estd::numeric_limits<Numeric>::template length<base>::value + 1;
    char_type buffer[N];

    // Works, just a touch too bloat-y still
    //const to_chars_result result = num_put::put_integer_nofill(buffer, buffer + N, out, value);

    // FIX: Causes catastrophic crash
    //const to_chars_result result = internal::to_chars<base>(buffer, buffer + N, value,
    //    out.flags() & ios_base::uppercase);

    const to_chars_result result = detail::to_chars<base>(buffer, buffer + N, value);
    const unsigned sz = &buffer[N] - result.ptr;

    // DEBT: Need to check to_chars_result error code
    //switch(result.ec)

    write_filled_buffer(out, result.ptr, sz);

    return out;
}
#endif

template <class Streambuf, class Base, typename Int>
detail::basic_ostream<Streambuf, Base>& out_int_helper(
    detail::basic_ostream<Streambuf, Base>& out, const Int& value)
{
    using policy = typename Base::policy_type;
    using locale_type = typename policy::locale_type;
    using char_type = typename remove_cvref<Streambuf>::type::char_type;
    using num_put = internal::integer_put<locale_type>;

    // base 8 for biggest possible string
    // +1 for potential - sign
    // +0 for null terminator, none required
#if FEATURE_ESTD_OSTREAM_OCTAL
    constexpr unsigned N = estd::numeric_limits<Int>::template length<8>::value + 1;
#else
    constexpr unsigned N = estd::numeric_limits<Int>::template length<10>::value + 1;
#endif
    char_type buffer[N];

    const to_chars_result result = num_put::to_chars(buffer, buffer + N, out, value);

    if(result.ec == 0)
    {
        const unsigned sz = &buffer[N] - result.ptr;
        write_filled_buffer(out, result.ptr, sz);
    }
    else
        out.setstate(ios_base::badbit);

    return out;
}

#if UNUSED
template <class Streambuf, class Base, typename Int>
detail::basic_ostream<Streambuf, Base>& out_int_helper_old(
    detail::basic_ostream<Streambuf, Base>& out, Int value)
{
    // DEBT: another typical enum -> traits/template conversion - a framework
    // support for that really would be useful
    switch(out.flags() & ios_base::basefield)
    {
#if FEATURE_ESTD_OSTREAM_OCTAL
        case ios_base::oct:
            return write_int<8>(out, value);
#endif

        case ios_base::dec:
            return write_int<10>(out, value);

        case ios_base::hex:
            return write_int<16>(out, value);

        default:
            // TODO: assert or log an error condition
            return out;
    }
}
#endif
}

}