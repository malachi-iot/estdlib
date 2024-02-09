#pragma once

#include "fwd.h"
#include "cbase.h"
#include "numpunct.h"

#include "../charconv.hpp"
#include "iterated/num_put.h"

#if FEATURE_ESTD_DRAGONBOX
// NOTE: Would gently prefer to isolate dragonbox away from being generally
// incuded at the num_put level - but to handle all different Iter types
// sorta requires we put this here
#include "dragonbox/dragonbox.h"
#endif


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
    template <unsigned base, class T>
    static iter_type put_integer(iter_type out, const ios_base& str, char_type fill, T value)
    {
        constexpr unsigned N = estd::numeric_limits<T>::template length<base>::value;

        // +1 for potential - sign
        // No extra space for null terminator, not needed for iter_type/stream out
        char buffer[N + 1];

        to_chars_result result = to_chars_opt(buffer, buffer + N, value, base);

        return copy(result.ptr, &buffer[N + 1], out);
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
#if __cpp_static_assert
        static_assert(false, "floating point not yet supported");
#endif
        return out;
    }
};

}
}
