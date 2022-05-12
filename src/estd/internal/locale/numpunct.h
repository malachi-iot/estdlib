#pragma once

#include "fwd.h"
#include "facet.h"
#include <estd/string.h>

namespace estd { namespace experimental {


// DEBT: Only to avoid confusion with regular internal not in experimental space
// Classes will be moved around and this namespace will go away
namespace _internal {

template <typename TChar>
struct numpunct_base;

template <>
struct numpunct_base<char>
{
    typedef char char_type;

    static char_type thousands_sep() { return ','; }
    static estd::layer2::const_string grouping() { return ""; }
};

}

template <>
struct numpunct<char, locale<locale_code::en_US, internal::encodings::UTF8> > :
    _internal::numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "true"; }
    static estd::layer2::const_string falsename() { return "false"; }
};


template <>
struct numpunct<char, locale<locale_code::fr_FR, internal::encodings::UTF8> > :
    _internal::numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "vrai"; }
    static estd::layer2::const_string falsename() { return "faux"; }
};


template <class TChar, class TLocale>
struct use_facet_helper4<numpunct<TChar, void>, TLocale>
{
    typedef numpunct<TChar, TLocale> facet_type;
    
    inline static numpunct<TChar, TLocale> use_facet(TLocale)
    {
        return numpunct<TChar, TLocale>();
    }
};

}}