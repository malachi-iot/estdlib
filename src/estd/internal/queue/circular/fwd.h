#pragma once

#include "enum.h"
#include "../../feature/queue.h"
#include "../../container/fwd.h"

#if __cpp_lib_concepts
#include <concepts>
#endif

#if __cpp_concepts
namespace estd::concepts::inline v1 {

template  <class T>
concept CircularQueuePolicy = requires
{
    T::type;
    typename T::container_type;
    requires IndexedContainer<typename T::container_type>;
#if __cpp_lib_concepts
    { T::options } -> std::convertible_to<internal::queue_options>;
#endif
};

}
#endif

namespace estd { namespace internal {

template <class Policy>
class circular_queue;

template <class Policy, class Enabled = void>
class circular_queue_impl;

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

template <class T, internal::queue_options o = ring_options::default_opt>
using ring = internal::circular_queue<internal::span_circular_policy<T, detail::dynamic_extent::value, o>>;

}

}
