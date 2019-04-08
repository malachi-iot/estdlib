#pragma once

#include "../limits.h"
// mainly just for intellisense, normally you don't include internal/promoted_type directly
#include "../type_traits.h"

// does what common_type does, but less aggressively promotes type
// this instead finds the nearest, smallest type to satisfy both sides
// brute forcing implementation for the short term
// (common_type it's not unusual for an int8_t to get pushed to a
//  int32_t - clearly it's meant for temporary/immediate arithmetic
//  scenarios)
// common_type has been very rigidly locked down in its defined behavior,
// so we aren't gonna modify it
//template <class T1, class T2>
//struct promoted_type;
namespace estd {
namespace internal {

template<class T>
struct promote_type;

template<>
struct promote_type<int8_t>
{
    typedef int16_t type;
};

template<>
struct promote_type<int16_t>
{
    typedef int32_t type;
};

template<>
struct promote_type<int32_t>
{
    typedef int64_t type;
};

// Can't promote past 64 bit, but some of the conditional-template logic
// touches the struct so we do need it
template<>
struct promote_type<int64_t>
{
    typedef void type;
};

template<>
struct promote_type<uint8_t>
{
    typedef uint16_t type;
};

template<>
struct promote_type<uint16_t>
{
    typedef uint32_t type;
};

template<>
struct promote_type<uint32_t>
{
    typedef uint64_t type;
};

}

// TODO: Fix name - auto_promote means, if necessary, move
// to a higher bitness than T1 or T2 (for signed/unsigned mixing)
template<class T1, class T2, bool auto_promote = true>
struct promoted_type
{
    typedef typename estd::conditional<
            estd::numeric_limits<T1>::digits >=
            estd::numeric_limits<T2>::digits,
            T1,
            T2>::type more_bits_type;

    typedef typename estd::conditional<
            estd::numeric_limits<T1>::digits<
                    estd::numeric_limits<T2>::digits,
                    T1,
                    T2>::type less_bits_type;

    // if the bigger type is unsigned and the smaller type is signed,
    // turn the bigger type into signed.  Otherwise:
    // big signed + small signed = big signed
    // big unsigned + small unsigned = big unsigned
    // big signed + small unsigned = big signed
    // NOTE: exceptional case when say a uint16_t and a int16_t are presented
    // in that case we need to detect both have the same bitness and a promotion
    // to int32_t may be needed
    typedef typename estd::conditional<
            !estd::numeric_limits<more_bits_type>::is_signed &&
            estd::numeric_limits<less_bits_type>::is_signed,
            typename estd::make_signed<more_bits_type>::type,
            more_bits_type>::type aligned_more_bits_type;

    // if less_bits_type is signed, more_bits_type is unsigned
    // and after adjusting both digits are the same then we risk
    // losing precision in more_bits_type when we subtract the bit
    // so promote to the next precision of type
    // NOTE: in that case, aligned_more_bits_type will (I think)
    // be identical to less_bits_type
    typedef typename estd::conditional<
            (estd::numeric_limits<aligned_more_bits_type>::digits ==
             estd::numeric_limits<less_bits_type>::digits) &&
            estd::numeric_limits<less_bits_type>::is_signed &&
            !estd::numeric_limits<more_bits_type>::is_signed &&
            auto_promote,
            typename internal::promote_type<aligned_more_bits_type>::type,
            aligned_more_bits_type>::type type;
};
}
