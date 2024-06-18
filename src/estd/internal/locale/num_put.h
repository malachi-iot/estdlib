#pragma once

#include "fwd.h"
#include "cbase.h"
#include "numpunct.h"

#include "../charconv.hpp"
#include "iterated/num_put.h"

namespace estd {

// In development, not ready, so marked as internal
namespace internal {

//constexpr
inline unsigned get_base2(const ios_base::fmtflags basefield)
{
    // DEBT: Consider a switch statement because we can, but might be slightly fragile
    if(basefield == ios_base::hex)
    {
        return 16;
    }
    else if(basefield == ios_base::dec)
    {
        return 10;
    }
    else
    {
        return 8;
        // oct
        // basefield mask is guaranteed to produce one of these 3 options
        // DEBT: Ensure there's a unit test to that effect elsewhere
    }
}

constexpr unsigned get_base(const ios_base::fmtflags basefield)
{
    return basefield == ios_base::hex ? 16 :
        basefield == ios_base::dec ? 10 :
        8;
}

template <class Char, class OutputIt, class Locale = classic_locale_type>
class num_put
{
public:
    typedef Char char_type;
    typedef OutputIt iter_type;

    // DEBT: Decouple from cbase_utf since this is ignoring incoming Locale
    template <unsigned b>
    using cbase_type = cbase_utf<Char, b, CBASE_DYNAMIC>;

    // DEBT: Low level helper - not too problematic, just clumsy name and location
    // leverages our cbase awareness.  Beware! uses _opt variety meaning that
    // *start* of str is at to_chars_result::ptr
    template <class T>
    static to_chars_result put_integer_nofill(iter_type first, iter_type last,
        const ios_base& str,
        const T& value)
    {
        const unsigned base = get_base(str.flags() & ios_base::basefield);

        if(base <= 10)
        {
            return internal::to_chars_integer_opt(
                first, last, value, internal::base_provider<>(base),
                cbase_type<10>{});
        }
        else
        {
            const bool uppercase = str.flags() & ios_base::uppercase;

            return internal::to_chars_integer_opt(
                first, last, value, internal::base_provider<>(base),
                cbase_type<36>(uppercase ? CBASE_UPPER : CBASE_LOWER));
        }
    }


private:
    template <class T>
    static iter_type put_integer(iter_type out, const ios_base& str,
        char_type fill,
        const T& value)
    {
        // Hardcode to base 8 since that's the biggest version
        // +1 for potential - sign
#if FEATURE_ESTD_OSTREAM_OCTAL
        constexpr unsigned N = estd::numeric_limits<T>::template length<8>::value + 1;
#else
        constexpr unsigned N = estd::numeric_limits<T>::template length<10>::value + 1;
#endif

        //const unsigned base = get_base(str.flags() & ios_base::basefield);

        // No extra space for null terminator, to_chars_opt gives us start/end ptrs
        char_type buffer[N];

        to_chars_result result = put_integer_nofill(buffer, buffer + N, str, value);

        unsigned width = str.width();
        const unsigned result_width = buffer + N - result.ptr;

        if(width > result_width)
        {
            width -= result_width;

            const ios_base::fmtflags adj = str.flags() & ios_base::adjustfield;
            if(adj == ios_base::right)
            {
                out = fill_n(out, width, fill);
                return copy(result.ptr, buffer + N, out);
            }
            else if(adj == ios_base::left)
            {
                out = copy(result.ptr, buffer + N, out);
                return fill_n(out, width, fill);
            }
            else
            {
                // DEBT: 'internal' flavor, needs work
                return out;
            }
        }
        else
            return copy(result.ptr, buffer + N, out);
    }

    template <unsigned base, class T>
    static iter_type put_float(iter_type out, const ios_base& str, char_type fill, T value)
    {
#if __cpp_static_assert
        static_assert(!is_floating_point<T>::value, "floating point not yet supported");
#endif
        return out;
    }

public:
    static iter_type put(iter_type out, const ios_base& str, char_type fill,
        bool value)
    {
        if(str.flags() & ios_base::boolalpha)
        {
            using np = numpunct<char_type, Locale>;
            // DEBT: We may prefer plain old const char* instead
            layer2::const_string _v = value ? np::truename() : np::falsename();
            const char* v = _v.data();
            while(*v != 0) *out++ = *v++;
        }
        else
        {
            using cb = cbase<char_type, 2, Locale>;
            *out++ = cb::to_char(value);
            return ++out;
        }

        return out;
    }

    template <class T>
    // type filter not actually needed at the moment, but will probably come in handy when
    // floats get involved
    static constexpr typename enable_if<numeric_limits<T>::is_integer, iter_type>::type
    //static iter_type
    put(iter_type out, const ios_base& str, char_type fill, T value)
    {
        return put_integer(out, str, fill, value);
    }

    template <class T>
    // type filter not actually needed at the moment, but will probably come in handy when
    // floats get involved
    static typename enable_if<is_floating_point<T>::value, iter_type>::type
    //static iter_type
    put(iter_type out, const ios_base& str, char_type fill, T value)
    {
        return put_float(out, str, fill, value);
    }
};

}
}
