#pragma once

#include <estd/string.h>

#include "fwd.h"
#include "facet.h"
#include "numpunct.h"

namespace estd {

namespace internal {

template <class Char, bool international>
struct ascii_us_moneypunct;

template<>
struct ascii_us_moneypunct<char, false>
{
    static constexpr estd::layer2::const_string curr_symbol() { return "$"; }
};


template<>
struct ascii_us_moneypunct<char, true>
{
    static constexpr estd::layer2::const_string curr_symbol() { return "USD "; }
};


// DEBT: Consider pulling in internal::classic_numpunct
template <class Char, bool international, class Locale>
struct moneypunct<Char, international, Locale,
    enable_if_t<is_compatible_with_classic_locale<Locale>::value>> :
    ascii_us_moneypunct<Char, international>,
    classic_numpunct<Char>
{
};

template <class Char, bool international, class Locale>
struct use_facet_helper<estd::moneypunct<Char, international, void>, Locale>
{
    using facet_type = estd::moneypunct<Char, international, Locale>;

    constexpr static facet_type use_facet(Locale)
    {
        return facet_type();
    }
};

}


}