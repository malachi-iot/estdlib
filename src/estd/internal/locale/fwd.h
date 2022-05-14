#pragma once

#include "../locale.h"
#include "../../variant.h"

namespace estd {

// DEBT: Make TImpl void again once we fix up helper to deal with that
template <typename TChar, class TLocale = void, class TImpl = estd::monostate>
class ctype;

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}