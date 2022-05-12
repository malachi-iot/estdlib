#pragma once

#include "fwd.h"

namespace estd { namespace experimental {

template <class TFacet, class TLocale>
struct use_facet_helper4;

template <class TFacet, class TLocale>
TFacet use_facet4(TLocale l)
{
    return use_facet_helper4<TFacet, TLocale>::use_facet(l);
}

}} // namespace estd { namespace experimental {