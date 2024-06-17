/**
 * These forward declarations are particularly important due to the heavily
 * specialization nature of our locale code.
 *
 * Even though one may be tempted to use alias template 'using' instead of
 * old-style inheritance, it is the latter that works better with specialization.
 */
#pragma once

#include "iso.h"
#include "../text/encodings.h"

namespace estd {

namespace internal {

// lifted from https://stackoverflow.com/questions/9510514/integer-range-based-template-specialisation
// DEBT: Put this into a fwd somewhere
template<bool>
struct Range;

enum cbase_casing
{
    CBASE_LOWER,    // fixed lower case
    CBASE_UPPER,    // fixed upper case
    CBASE_DYNAMIC   // runtime selectable
};

template <typename Char, unsigned b, cbase_casing casing = CBASE_LOWER, typename = estd::internal::Range<true> >
struct cbase_utf;

template <typename Char, unsigned b, class Locale, typename Enabled = void>
struct cbase;

template <class Char, class Locale, typename TEnabled = void>
class ctype;

template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale;

template <typename Char, class Locale, class Enabled = void>
struct numpunct;

}

namespace iterated {

template <unsigned base, typename Char, class Locale>
struct num_get;

}

template <typename Char, unsigned b, class Locale = void>
using cbase = internal::cbase<Char, b, Locale>;



template <typename Char, class Locale = void>
class ctype;

template <class Char, class Locale = void>
struct numpunct;

template <class Char, bool international = false, class Locale = void>
struct moneypunct;


}