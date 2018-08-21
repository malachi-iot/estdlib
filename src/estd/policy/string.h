#pragma once

#include "../traits/string.h"

// TODO: traits/string.h at time of this writing actually *only* has policies (some mislabeled as traits)
//       so move them into here once FEATURE_ESTD_STRICT_DYNAMIC_ARRAY migration is complete

namespace estd {

// prototype
template<
    class CharT,
    class Traits,
    class Allocator,
    class Policy
> class basic_string;

template <class CharT,
          class Traits,
          class Policy = experimental::sized_string_policy<Traits, int16_t, true> >
class basic_string_view;


}

