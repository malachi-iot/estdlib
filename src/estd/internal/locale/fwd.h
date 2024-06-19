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

enum cbase_policies : unsigned
{
    CBASE_POLICY_CASE_DYNAMIC = 0,  // runtime selectable uppercase or lowercase
    CBASE_POLICY_CASE_LOWER,  // fixed lower case
    CBASE_POLICY_CASE_UPPER,  // fixed upper case

    CBASE_POLICY_CASE_MASK = 0x03,

    // If true, treats all int -> string conversions with hex capability.
    // Can save ROM space in scenarios where both hex and non-hex conversions are used
    // Incurs slight performance penalty for non-hex conversions
    CBASE_POLICY_HEX_ALWAYS = 0x04,

    CBASE_POLICY_DEFAULT = CBASE_POLICY_CASE_DYNAMIC,
};

using cbase_casing = cbase_policies;

template <typename Char, unsigned b, cbase_policies policy, typename = estd::internal::Range<true> >
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