#pragma once

#include "fwd.h"
#include "facet.h"

#include "../raw/type_traits.h"

namespace estd { namespace internal {

// Can the presented encoding work with the core encoding?  If so we get a 'type'
// of the presented encoding
// DEBT: Naming should be changed to reflect demote/promote relation between the two encodings
template<internal::encodings core_encoding, internal::encodings presented_encoding,
    class T = void>
struct is_compatible_encoding : estd::false_type {};

template<class T>
struct is_compatible_encoding<internal::encodings::ASCII, internal::encodings::UTF8, T> :
    estd::true_type
{
    typedef T type;
};

template<class T>
struct is_compatible_encoding<internal::encodings::ASCII, internal::encodings::ASCII, T> :
    estd::true_type
{
    typedef T type;
};

// locale codes are sometimes freely interchangeable.  For example, numpunct true/false is identical
// for en_US and en_GB and all other english variants.  For those scenarios, use this helper
template<locale_codes core, locale_codes presented,
    class T = void>
struct is_compatible_locale_code : estd::false_type {};


template<class T>
struct is_compatible_locale_code<locale_codes::en_US, locale_codes::en_GB, T> :
    estd::true_type
{
    typedef T type;
};

template<class T>
struct is_compatible_locale_code<locale_codes::en_US, locale_codes::en_US, T> :
    estd::true_type
{
    typedef T type;
};


template<class T>
struct is_compatible_locale_code<locale_codes::en_US, locale_codes::C, T> :
    estd::true_type
{
    typedef T type;
};

template<class Locale, class T = void>
struct is_compatible_with_classic_locale : estd::false_type {};

template <locale_codes lc, internal::encodings encoding>
struct is_compatible_with_classic_locale<locale<lc, encoding>,
    estd::enable_if_t<
        is_compatible_encoding<internal::encodings::ASCII, encoding>::value &&
        is_compatible_locale_code<locale_codes::en_US, lc>::value>> :
    estd::true_type
{
    //typedef bool type;
};

}}
