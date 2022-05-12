#pragma once

#include "../locale.h"

namespace estd { namespace experimental {

template <class TChar, class TLocale = void>
struct numpunct;

template <class TChar, bool international = false, class TLocale = void>
struct moneypunct;


}}