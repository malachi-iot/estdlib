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

template<class T>
using promote_type_t = typename promote_type<T>::type;

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

template<>
struct promote_type<int64_t>
{
#if __SIZEOF_INT128__
    using type = __int128_t;
#else
    // Can't promote past 64 bit, but some of the conditional-template logic
    // touches the struct so we do need it
    typedef void type;
#endif
};

// clang's typedef/aliasing of int64_t is not interchangeable with long long
// 31AUG26 MB DEBT: Probably need this for int, long, etc. also
#if __clang__ && __SIZEOF_LONG_LONG__ == 8
template<>
struct promote_type<long long> : promote_type<int64_t>  {};
#endif

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

template<>
struct promote_type<uint64_t>
{
#if __SIZEOF_INT128__
    using type = __uint128_t;
#else
    // Can't promote past 64 bit, but some of the conditional-template logic
    // touches the struct so we do need it
    typedef void type;
#endif
};

#if __clang__ && __SIZEOF_LONG_LONG__ == 8
template<>
struct promote_type<unsigned long long> : promote_type<uint64_t>  {};
#endif

template<>
struct promote_type<float>
{
    using type = double;
};

// 01SEP26 DEBT: I cannot account for this.  int is definitely 4 bytes wide on ESP32, yet
// int32 specialization doesn't pick this up.  No need for a short/long flavor, those
// do get picked up by int16_t.  Presumably this has something to do with long being
// same size as int
#if ESP_PLATFORM
template<>
struct promote_type<int>
{
    using type = long;
};
#endif

// 01SEP26 DEBT: No actual guarantee that our stdint specializations match up to
// int -> long.  But so far so good
static_assert(is_same<promote_type_t<int>, long>::value, "Cannot promote int -> long");

#if __SIZEOF_INT128__ && __SIZEOF_LONG_LONG__ == 8
// FIX: Some trouble here, we can't yet tell if platform 100% aliases int64_t to long long or not.  We know Clang doesn't,
// but it seems like GCC does sometimes and doesn't others.  Or perhaps it's limited to __int128_t itself?
//static_assert(is_same<promote_type_t<long long>, __int128_t>::value);
#endif


}

// DEBT: Fix name - auto_promote means, if necessary, move
// to a higher bitness than T1 or T2 for signed/unsigned mixing
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
            internal::promote_type_t<aligned_more_bits_type>,
            aligned_more_bits_type>::type type;
};

// DEBT: Crude application for non-integers
template<bool auto_promote>
struct promoted_type<float, double, auto_promote>
{
    using type = double;
};


// AI provided narrowing sanity check, somewhat works
template<class From, class To, class = void>
struct is_safe_arithmetic_conversion : false_type {};

template<class From, class To>
struct is_safe_arithmetic_conversion<
    From,
    To,
    decltype(void(To{ std::declval<From>() }))
    > : true_type
{
};

// Human provided additional support
// Permits signed -> unsigned provided 'To' is already a promoted precision
template <class From, class To>
struct is_safe_arithmetic_conversion<From, To,
    enable_if_t<!is_same<From, To>::value && !is_signed<To>::value>> :
    bool_constant<is_safe_arithmetic_conversion<From, internal::promote_type_t<From>>::value>
{
};

template <> struct is_safe_arithmetic_conversion<short, float> : true_type {};
template <> struct is_safe_arithmetic_conversion<short, double> : true_type {};
template <> struct is_safe_arithmetic_conversion<int, double> : true_type {};

static_assert(is_safe_arithmetic_conversion<int, double>::value, "Sanity check");
static_assert(is_safe_arithmetic_conversion<float, double>::value, "Sanity check");
static_assert(!is_safe_arithmetic_conversion<double, float>::value, "Sanity check");
static_assert(is_safe_arithmetic_conversion<int, long>::value, "Sanity check");
static_assert(!is_safe_arithmetic_conversion<int, short>::value, "Sanity check");
static_assert(is_safe_arithmetic_conversion<short, unsigned long>::value, "Sanity check");
static_assert(!is_safe_arithmetic_conversion<unsigned long, int>::value, "Sanity check");
static_assert(!is_safe_arithmetic_conversion<unsigned long, float>::value, "Sanity check");

}
