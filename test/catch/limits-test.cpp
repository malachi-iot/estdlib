#include <catch.hpp>

#include <estd/chrono.h>
#include <estd/limits.h>

#include <estd/functional.h>

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
struct promote_type<uint8_t>
{
    typedef uint16_t type;
};

template<>
struct promote_type<uint16_t>
{
    typedef uint32_t type;
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

using namespace estd;

TEST_CASE("limits & common_type tests")
{
    // https://stackoverflow.com/questions/15211463/why-isnt-common-typelong-unsigned-longtype-long-long
    // summarizes for us that common_type will not go to something like an int64_t - instead it is going
    // to choose one of the two
    SECTION("common_type")
    {
        SECTION("signed test 1")
        {
            typedef typename common_type<int16_t, uint32_t>::type common_type;

            //int sz = sizeof(common_type);

            //REQUIRE(sz == 4);
            //REQUIRE((numeric_limits<common_type>::digits) == 32);
            //auto max = numeric_limits<common_type>::max();
            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be unsigned
            REQUIRE(!is_signed);
        }
        SECTION("signed test 2")
        {
            typedef typename common_type<uint16_t, int32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 31);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 3")
        {
            typedef typename common_type<int32_t, uint32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(!is_signed);
        }
        SECTION("signed test 4")
        {
            typedef typename common_type<int8_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            // FIX: This has a problem, digits reports 31 here
            // verified decay itself is working OK
            //REQUIRE(digits == 15);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 5")
        {
            typedef typename common_type<int16_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            // FIX: This has a problem, digits reports 31 here
            //REQUIRE(digits == 15);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 6")
        {
            typedef typename common_type<int64_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            // FIX: This has a problem, digits reports 31 here
            REQUIRE(digits == 63);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("internal::cond_t")
        {
            typedef decltype (std::declval<int16_t>()) t1;
            typedef decltype (std::declval<uint8_t>()) t2;
            // It seems that this statement always results in a 32-bit type
            typedef decltype (false ? std::declval<int16_t>() : std::declval<uint8_t>()) t3;

            int digits;

            // Interestingly we have to decay t1 and t2 as they come out to be &&
            digits = numeric_limits<estd::decay_t<t1> >::digits;

            REQUIRE(digits == 15);

            digits = numeric_limits<estd::decay_t<t2> >::digits;

            REQUIRE(digits == 8);

            digits = numeric_limits<estd::decay_t<t3> >::digits;

            //REQUIRE(digits == 15);

            typedef internal::cond_t<int16_t, uint8_t> cond_type;

            digits = numeric_limits<cond_type>::digits;

            // FIX: problem localizes here
            //REQUIRE(digits == 15);
        }
    }
    SECTION("8 bit")
    {
        auto digits = numeric_limits<int8_t>::digits;

        REQUIRE(digits == 7);

        digits = numeric_limits<uint8_t>::digits;

        REQUIRE(digits == 8);
    }
    SECTION("16 bit")
    {
        auto digits = numeric_limits<int16_t>::digits;

        REQUIRE(digits == 15);

        digits = numeric_limits<uint16_t>::digits;

        REQUIRE(digits == 16);
    }
    SECTION("32 bit")
    {
        auto digits = numeric_limits<int32_t>::digits;

        REQUIRE(digits == 31);

        digits = numeric_limits<uint32_t>::digits;

        REQUIRE(digits == 32);
    }
    SECTION("promote_type")
    {
        SECTION("1:1")
        {
            typedef typename promoted_type<uint8_t, uint8_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 8);
        }
        SECTION("simple")
        {
            typedef typename promoted_type<int8_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
        SECTION("uint8_t -> int16_t")
        {
            typedef typename promoted_type<uint8_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
        SECTION("int16_t -> uint16_t (auto promote to int32_t)")
        {
            // We promote to int32_t by default to avoid precision loss
            typedef typename promoted_type<uint16_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 31);
        }
        SECTION("int16_t -> uint16_t (permit 'promoting' to same bitness int16_t)")
        {
            // We shut off auto-promote for this, indicating we are confident
            // no precision loss going from uint16_t -> int16_t
            typedef typename promoted_type<uint16_t, int16_t, false>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
    }
}
