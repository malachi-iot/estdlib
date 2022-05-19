#pragma once

#include "fwd.h"

namespace estd { namespace internal {

template <class TFacet, class TLocale>
struct use_facet_helper;

}

template <class TFacet, class TLocale>
typename internal::use_facet_helper<TFacet, TLocale>::facet_type use_facet(TLocale l)
{
    return internal::use_facet_helper<TFacet, TLocale>::use_facet(l);
}


} // namespace estd { namespace experimental {