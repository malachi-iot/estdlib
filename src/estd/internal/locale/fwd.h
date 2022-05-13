#pragma once

#include "../locale.h"

namespace estd {

template <typename TChar, class TLocale = void>
class ctype;

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}