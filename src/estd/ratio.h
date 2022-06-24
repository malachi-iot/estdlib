#pragma once

#include "internal/platform.h"
#include "type_traits.h"
#include "cstdint.h"

namespace estd {

template<
    std::intmax_t Num,
    std::intmax_t Denom = 1
> class ratio
{
public:
    typedef ratio type;

    static CONSTEXPR std::intmax_t num = Num;
    static CONSTEXPR std::intmax_t den = Denom;
};

typedef ratio<1, 1000000000000> pico;
typedef ratio<1, 1000000000> nano;
typedef ratio<1, 1000000> micro;
typedef ratio<1, 1000> milli;
typedef ratio<1, 100> centi;
typedef ratio<1, 10> deci;
typedef ratio<10, 1> deca;
typedef ratio<1000, 1> kilo;
typedef ratio<1000000, 1> mega;
typedef ratio<1000000000, 1> giga;
typedef ratio<1000000000000, 1> tera;

namespace detail {

template <class R1, class R2>
struct ratio_divide;

template <class R1, class R2>
struct ratio_multiply;

}

namespace internal {

// https://stackoverflow.com/questions/43846187/using-template-metaprogramming-in-c-find-the-gcd-of-two-integers
template<std::intmax_t a, std::intmax_t b> struct gcd
{
    static CONSTEXPR std::intmax_t value = gcd<b, a % b>::value;
};

template<std::intmax_t a>
struct gcd<a, 0>
{
    static CONSTEXPR std::intmax_t value = a;
};

// https://www.variadic.xyz/2012/01/07/c-template-metaprogramming/
template<std::intmax_t One, std::intmax_t Two>
struct lcm
{
    enum
    {
        value = One * Two / gcd<One, Two>::value
    };
};


}


/*
// both have numerator of 1
template<std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

// TODO: Make a beefier version which queries numeric_limits

/*
// second has a numerator of 1, first has variable numerator
template<std::intmax_t Num1, std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<Num1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2 * Num1;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

/*
// second has a numerator of 1, first has variable numerator
template<std::intmax_t Num1, std::intmax_t Denom1, std::intmax_t Denom2,
         class = typename estd::enable_if<
             //((Denom2 * Num1) % Denom1) == 0
             true
             > >
struct ratio_divide<ratio<Num1, Denom1>, ratio<1, Denom2> >
{
    static CONSTEXPR std::intmax_t num = Denom2 * Num1;
    static CONSTEXPR std::intmax_t den = Denom1;
}; */

namespace detail {

// TODO: Need to do overflow processing
template <std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_divide<ratio<Num1, Denom1>, ratio<Num2, Denom2> >
{
private:
    static CONSTEXPR std::intmax_t gcd =
        internal::gcd<Denom2 * Num1, Denom1 * Num2>::value;

public:
    static CONSTEXPR std::intmax_t num = Denom2 * Num1 / gcd;
    static CONSTEXPR std::intmax_t den = Denom1 * Num2 / gcd;

    typedef estd::ratio<num, den> type;
};


template <std::intmax_t Num1, std::intmax_t Num2,
    std::intmax_t Denom1, std::intmax_t Denom2>
struct ratio_multiply<ratio<Num1, Denom1>, ratio<Num2, Denom2> >
{
private:
    static CONSTEXPR std::intmax_t gcd =
        internal::gcd<Denom2 * Denom1, Num1 * Num2>::value;

public:
    static CONSTEXPR std::intmax_t num = Num1 * Num2 / gcd;
    static CONSTEXPR std::intmax_t den = Denom1 * Denom2 / gcd;

    typedef estd::ratio<num, den> type;
};

}

template <class R1, class R2>
using ratio_multiply = typename detail::ratio_multiply<R1, R2>::type;

template <class R1, class R2>
using ratio_divide = typename detail::ratio_divide<R1, R2>::type;


}
