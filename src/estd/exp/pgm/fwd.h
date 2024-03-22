#pragma once

#include <estd/internal/fwd/variant.h>

namespace estd {

namespace internal {

namespace impl {

template <class T, bool near = true>
struct pgm_accessor_impl;

template <class T, size_t N>
struct pgm_allocator_traits;

enum class PgmPolicyType
{
    Buffer,         // layer2-class buffer
    String,
    BufferInline    // layer1-class buffer
};

// type=String, N = max = null terminated
// type=String, N != max = fixed compile time size
// type=Buffer, N = max = runtime sized
// type=Buffer, N != max = fixed compile time size
template <class T = char, PgmPolicyType type_ = PgmPolicyType::String,
    size_t N = estd::internal::variant_npos()>
struct PgmPolicy;

}

}

inline namespace v0 { inline namespace avr {

namespace impl {

template <class T, size_t N, class Policy>
struct pgm_array;

template <class T, unsigned N>
using pgm_string = pgm_array<T, N, internal::impl::PgmPolicy<
    T, internal::impl::PgmPolicyType::String, N> >;

}

} }

}


// Experimenting with strlen-less end() code, but it seems to gain nothing
// really
#define FEATURE_ESTD_PGM_EXP_IT 0

// Dogfooding in allocator.  So far it just seems to make it more complicated,
// but a feeling tells me this will be useful
#define FEATURE_ESTD_PGM_ALLOCATOR 1

// TODO: Use __WITH_AVRLIBC__ in features area


