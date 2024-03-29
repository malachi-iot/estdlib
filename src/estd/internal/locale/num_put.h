#pragma once

#include "fwd.h"
#include "cbase.h"
#include "numpunct.h"

#include "../charconv.hpp"
#include "iterated/num_put.h"

namespace estd {

// In development, not ready, so marked as internal
namespace internal {

template <class TChar, class OutputIt, class TLocale = classic_locale_type>
class num_put
{
public:
    typedef TChar char_type;
    typedef OutputIt iter_type;

private:
    // DEBT: Still need to do 'fill'
    template <unsigned base, class T>
    static iter_type put_integer(iter_type out, const ios_base& str, char_type fill, T value)
    {
        // +1 for potential - sign
        constexpr unsigned N = estd::numeric_limits<T>::template length<base>::value + 1;

        // No extra space for null terminator, not needed for iter_type/stream out
        char buffer[N];

        to_chars_result result = to_chars_opt(buffer, buffer + N, value, base);

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
        using np = numpunct<char_type, TLocale>;
        // DEBT: We may prefer plain old const char* instead
        layer2::const_string _v = value ? np::truename() : np::falsename();
        const char* v = _v.data();
        while(*v != 0) *out++ = *v++;
        return out;
    }

    template <class T>
    // type filter not actually needed at the moment, but will probably come in handy when
    // floats get involved
    static typename enable_if<numeric_limits<T>::is_integer, iter_type>::type
    //static iter_type
    put(iter_type out, const ios_base& str, char_type fill, T value)
    {
        //const ios_base::fmtflags basefield = str.flags() & ios_base::basefield;
        constexpr unsigned base = 10;
        return put_integer<base>(out, str, fill, value);
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
