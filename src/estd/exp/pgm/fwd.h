#pragma once

#include <estd/internal/fwd/variant.h>

namespace estd {

namespace internal {

namespace impl {

template <class T, bool near = true>
struct pgm_accessor_impl;

template <class T, size_t N = internal::variant_npos()>
struct pgm_allocator_traits;

}

}

}


// Experimenting with strlen-less end() code, but it seems to gain nothing
// really
#define FEATURE_ESTD_PGM_EXP_IT 0

// Dogfooding in allocator.  So far it just seems to make it more complicated,
// but a feeling tells me this will be useful
#define FEATURE_ESTD_PGM_ALLOCATOR 1

// TODO: Use __WITH_AVRLIBC__ in features area


