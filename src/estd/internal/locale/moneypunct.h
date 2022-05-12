#pragma once

#include "fwd.h"
#include "facet.h"
#include <estd/string.h>

namespace estd { namespace experimental {

template <class TChar, bool international, class TLocale>
struct moneypunct;

template <>
struct moneypunct<char, false, locale<locale_code::en_US, internal::encodings::UTF8> >
{
    static char decimal_point() { return '.'; }

    static estd::layer2::const_string curr_symbol() { return "$"; }
};


template <>
struct moneypunct<char, true, locale<locale_code::en_US, internal::encodings::UTF8> >
{
    static char decimal_point() { return '.'; }

    static estd::layer2::const_string curr_symbol() { return "USD"; }
};


template <class TChar, bool international, class TLocale>
struct use_facet_helper4<moneypunct<TChar, international, TLocale>, TLocale>
{
    inline static moneypunct<TChar, international, TLocale> use_facet(TLocale)
    {
        return moneypunct<TChar, international, TLocale>();
    }
};



}}