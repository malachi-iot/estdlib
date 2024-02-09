#pragma once

#include "../cbase.h"
#include "../numpunct.h"
#include "../fwd.h"

namespace estd { namespace internal { namespace iterated {

template <class Numeric, typename Char, class Locale, class = void>
struct num_put;

template <typename Char, class Locale>
struct num_put<double, Char, Locale>
{

};

}}}