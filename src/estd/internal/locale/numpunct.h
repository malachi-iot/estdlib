#pragma once

#include "fwd.h"
#include "facet.h"
#include "utility.h"
#include <estd/string.h>

namespace estd {


// DEBT: Only to avoid confusion with regular internal not in experimental space
// Classes will be moved around and this namespace will go away
namespace internal {


template <typename TChar>
struct numpunct_base;

template <>
struct numpunct_base<char>
{
    typedef char char_type;

    static ESTD_CPP_CONSTEXPR_RET char_type thousands_sep() { return ','; }
    static estd::layer2::const_string grouping() { return ""; }
    static ESTD_CPP_CONSTEXPR_RET char_type decimal_point() { return '.'; }
};

//template <locale_code::values lc, internal::encodings::values encoding>
template <class TLocale>
struct numpunct<char, 
    //locale<lc, encoding>,
    TLocale,
    // TODO: Almost works, but not quite yet.  Figure out why.  Would be much nicer
    // than having to explicitly involve enable_if
    //typename is_compatible_with_classic_locale<TLocale>::type> :

    typename estd::enable_if<

        //(encoding == internal::encodings::ASCII || encoding == internal::encodings::UTF8) &&
        //(lc == locale_code::en_US || lc == locale_code::en_GB)>

        //is_compatible_encoding<internal::encodings::ASCII, encoding>::value &&
        //is_compatible_locale_code<locale_code::en_US, lc>::value>

        //is_compatible_with_classic_locale<locale<lc, encoding> >::value>
        internal::is_compatible_with_classic_locale<TLocale>::value>

        ::type> :
    internal::numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "true"; }
    static estd::layer2::const_string falsename() { return "false"; }
};


template <internal::encodings::values encoding>
struct numpunct<char,
    internal::locale<internal::locale_code::fr_FR, encoding>,
    typename internal::is_compatible_encoding<internal::encodings::ASCII, encoding>::type> :
    numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "vrai"; }
    static estd::layer2::const_string falsename() { return "faux"; }
};

}


// All this wrapping is done to hide the 'TEnabled' template portion.
// This comes in handy when doing custom specializations, one can avoid specifying
// that extra 3rd template parameter.
template <typename TChar, class TLocale>
struct numpunct : internal::numpunct<TChar, TLocale> {};


namespace internal {

template <class TChar, class TLocale>
struct use_facet_helper<estd::numpunct<TChar, void>, TLocale>
{
    typedef estd::numpunct<TChar, TLocale> facet_type;
    
    inline static facet_type use_facet(TLocale) { return facet_type(); }
};

}

}