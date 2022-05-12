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

template<bool>
struct Range;

// Can the presented encoding work with the core encoding?  If so we get a 'type'
// of the presented encoding
template<internal::encodings::values core_encoding, internal::encodings::values presented_encoding>
struct is_compatible_encoding;

// DEBT: Pretty sure this won't work in pre-C++11
template<>
struct is_compatible_encoding<internal::encodings::ASCII, internal::encodings::UTF8>
{
    static CONSTEXPR internal::encodings::values value = internal::encodings::UTF8;
    static CONSTEXPR internal::encodings::values _value() { return internal::encodings::UTF8; }
};



//template <internal::encodings::values encoding>
template <>
struct numpunct<char, 
    locale<locale_code::en_US, 
        /*
        is_compatible_encoding<
            internal::encodings::ASCII, encoding>
            ::_value()> */
            internal::encodings::UTF8>
        > :
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

/*
template <class TChar, locale_code::values lc>
struct use_facet_helper4<numpunct<TChar, void>, locale<lc, internal::encodings::UTF8> >
    : use_facet_helper4<numpunct<TChar, void>, locale<lc, internal::encodings::ASCII> > {}; */


}}