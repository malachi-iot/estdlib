#pragma once

#include "fwd.h"
#include "facet.h"
#include <estd/string.h>

namespace estd {

namespace internal {

// DEBT: Make an 'ascii' base class for moneypunct
// https://github.com/malachi-iot/estdlib/issues/142

template <class Char, bool international>
struct ascii_moneypunct;


template<>
struct ascii_moneypunct<char, false>
{

};


template<>
struct ascii_moneypunct<char, true>
{

};


template <class Char, class Locale, class Enabled = void>
struct moneypunct
{

};

}

template <>
struct moneypunct<char, false, internal::locale<internal::locale_code::en_US, internal::encodings::UTF8> >
{
    static char decimal_point() { return '.'; }

    static estd::layer2::const_string curr_symbol() { return "$"; }
};


template <>
struct moneypunct<char, true, internal::locale<internal::locale_code::en_US, internal::encodings::UTF8> >
{
    static char decimal_point() { return '.'; }

    static estd::layer2::const_string curr_symbol() { return "USD "; }
};


namespace internal {

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