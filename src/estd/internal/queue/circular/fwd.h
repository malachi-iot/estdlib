#pragma once

//#include "../../container/unordered/fwd.h"
#include "enum.h"

#if __cpp_lib_concepts
#include <concepts>
#endif

#if __cpp_concepts
namespace estd::concepts { inline namespace v1 {

template  <class T>
concept CircularQueuePolicy = requires
{
    T::type;
};

}}
#endif

namespace estd { namespace internal {

template <class Policy>
class circular_queue;

template <class Policy, class Enabled = void>
class circular_queue_impl;

//template <class T, queue_options o, class Nullable = nullable_traits<T>>
//struct circular_policy;

template <class T, size_t N, queue_options o>
struct array_circular_policy;

template <class T, size_t N, queue_options o>
struct span_circular_policy;

}}

namespace estd {

using ring_options = internal::queue_options;

namespace layer1 {

template <class T, unsigned N, internal::queue_options o = ring_options::default_opt>
using ring = internal::circular_queue<internal::array_circular_policy<T, N, o>>;

}

namespace layer2 {

template <class T, unsigned N, internal::queue_options o = ring_options::default_opt>
using ring = internal::circular_queue<internal::span_circular_policy<T, N, o>>;

}

namespace layer3 {

template <class T, unsigned N, internal::queue_options o = ring_options::default_opt>
using ring = internal::circular_queue<internal::span_circular_policy<T, detail::dynamic_extent::value, o>>;

}

}

// DEBT: Tossing in feature flag in this spot not ideal

#ifndef FEATURE_ESTD_GH144
// Nearly there, just need to sort out strict vs contract behavior
#define FEATURE_ESTD_GH144 1
#endif