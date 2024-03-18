#pragma once

#include "platform.h"

#include "macro/push.h"

#include "fwd/limits.h"
#include "type_traits.h"
#include "limits/macros.h"
#include "limits/integer.h"

namespace estd { namespace internal {

// NOTE: Did anyone ever hear of a non-signed floating point type?
template <class T, bool is_signed_ = true>
struct float_limits
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_signed = is_signed_;
    static CONSTEXPR unsigned radix = FLT_RADIX;

#if defined(__GCC_IEC_559) || defined(__STDC_IEC_559__)
    static CONSTEXPR bool is_iec559 = true;

#if FEATURE_ESTD_DRAGONBOX
    // NOTE: Shamelessly reworked from Dragonbox' max_output_string_length
    // DEBT: For 32 bit, is one byte too large due to exp len of 3 when it
    // should be 2
    template <class Format = jkj::dragonbox::default_float_traits<T> >
    struct length_exp :
        integral_constant<unsigned,
            // sign(1) + significand(9) + decimal_point + exp_marker + exp_sign + exp
            1 + Format::decimmal_digits + 1 + 1 + 3>::value >;
#endif

#else
    static CONSTEXPR bool is_iec559 = false;
#endif

};

}}

#include "macro/pop.h"

