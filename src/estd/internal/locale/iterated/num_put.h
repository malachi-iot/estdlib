#pragma once

#include "../cbase.h"
#include "../numpunct.h"
#include "../fwd.h"

#if FEATURE_ESTD_DRAGONBOX
// NOTE: Would gently prefer to isolate dragonbox away from being generally
// incuded at the num_put level - but to handle all different Iter types
// sorta requires we put this here
#include "dragonbox/dragonbox.h"


// DEBT: Move this out of here
namespace estd {

}
#endif


namespace estd { namespace internal { namespace iterated {

template <class Numeric, typename Char, class Locale, class = void>
struct num_put;

template <typename Char, class Locale>
struct num_put<double, Char, Locale>
{

};

}}}