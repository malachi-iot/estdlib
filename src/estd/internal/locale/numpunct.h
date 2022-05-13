#pragma once

#include "fwd.h"
#include "facet.h"
#include "utility.h"
#include <estd/string.h>

namespace estd {


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

template <typename TChar, class TLocale, class TEnabled = void>
struct numpunct;

//template <locale_code::values lc, internal::encodings::values encoding>
template <class TLocale>
struct numpunct<char, 
    //locale<lc, encoding>,
    TLocale,
    // Almost works, but not quite
    //typename is_compatible_with_classic_locale<TLocale>::type> :
    typename estd::enable_if<

        //(encoding == internal::encodings::ASCII || encoding == internal::encodings::UTF8) &&
        //(lc == locale_code::en_US || lc == locale_code::en_GB)>

        //is_compatible_encoding<internal::encodings::ASCII, encoding>::value &&
        //is_compatible_locale_code<locale_code::en_US, lc>::value>

        //is_compatible_with_classic_locale<locale<lc, encoding> >::value>
        internal::_internal::is_compatible_with_classic_locale<TLocale>::value>

        ::type> :
    _internal::numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "true"; }
    static estd::layer2::const_string falsename() { return "false"; }
};


template <internal::encodings::values encoding>
struct numpunct<char, 
    locale<internal::locale_code::fr_FR, encoding>,
    typename internal::_internal::is_compatible_encoding<internal::encodings::ASCII, encoding>::type> :
    _internal::numpunct_base<char>
{
    static estd::layer2::const_string truename() { return "vrai"; }
    static estd::layer2::const_string falsename() { return "faux"; }
};

}


// NOTE: All this wrapping is done to hide the 'TEnabled' template portion.
// That may be of little consequence overall though.  If so, merely expose
// the underlying numpunct and call it a day
#ifdef FEATURE_CPP_ALIASTEMPLATExx
// Conflicts with fwd declaration
template <typename TChar, class TLocale = void>
using numpunct = _internal::numpunct<TChar, TLocale>;
#else
template <typename TChar, class TLocale>
struct numpunct : _internal::numpunct<TChar, TLocale> {};
#endif


namespace internal {

template <class TChar, class TLocale>
struct use_facet_helper<numpunct<TChar, void>, TLocale>
{
    typedef numpunct<TChar, TLocale> facet_type;
    
    inline static facet_type use_facet(TLocale) { return facet_type(); }
};

}

}