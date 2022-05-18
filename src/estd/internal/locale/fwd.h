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

template <typename TChar, unsigned b, class TLocale, typename TEnabled = void>
struct cbase;

template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale;

template <typename TChar, class TLocale, class TEnabled = void>
struct numpunct;

}

namespace iterated {

template <unsigned base, typename TChar, class TLocale>
struct num_get;

}

template <typename TChar, unsigned b, class TLocale = void>
#ifdef FEATURE_CPP_ALIASTEMPLATE
using cbase = internal::cbase<TChar, b, TLocale>;
#else
struct cbase : internal::cbase<TChar, b, TLocale> {};
#endif



template <typename TChar, class TLocale = void>
class ctype;

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}