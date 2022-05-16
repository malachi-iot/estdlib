#pragma once

#include "iso.h"
#include "../text/encodings.h"

namespace estd {

namespace internal {

template <typename TChar, unsigned b, class TLocale, typename TEnabled = void>
struct cbase;

template <internal::locale_code::values locale_code, internal::encodings::values encoding>
struct locale;

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


// DEBT: Make TImpl void again once we fix up helper to deal with that
template <typename TChar, class TLocale = void>
class ctype;

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}