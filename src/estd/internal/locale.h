// declarations
#pragma once

namespace estd { namespace experimental {

template <class TChar>
class ctype;

struct locale;

template <class TFacet>
const TFacet& use_facet(const locale& loc);

}}