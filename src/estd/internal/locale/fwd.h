#pragma once

#include "../locale.h"
#include "../../variant.h"

namespace estd {

namespace internal {

template <typename TChar, unsigned b, class TLocale, typename TEnabled = void>
struct cbase;

}

template <typename TChar, unsigned b, class TLocale = void>
#ifdef FEATURE_CPP_ALIASTEMPLATE
using cbase = internal::cbase<TChar, b, TLocale>;
#else
struct cbase : internal::cbase<TChar, b, TLocale> {};
#endif


// DEBT: Make TImpl void again once we fix up helper to deal with that
template <typename TChar, class TLocale = void, class TImpl = estd::monostate>
class ctype;

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}