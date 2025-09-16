#pragma once

//#include "../../container/unordered/fwd.h"

namespace estd { namespace internal {

template <class Policy>
class circular_queue;

template <class Policy, class Enabled = void>
class circular_queue_base;

//template <class T, queue_options o, class Nullable = nullable_traits<T>>
//struct circular_policy;

template <class T, size_t N, queue_options o>
struct array_circular_policy;

template <class T, size_t N, queue_options o>
struct span_circular_policy;

}}

namespace estd {

namespace layer1 {

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using circular_queue = internal::circular_queue<internal::array_circular_policy<T, N, o>>;

}

namespace layer2 {

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using circular_queue = internal::circular_queue<internal::span_circular_policy<T, N, o>>;

}

namespace layer3 {

template <class T, unsigned N, internal::queue_options o = internal::queue_options::default_opt>
using circular_queue = internal::circular_queue<internal::span_circular_policy<T, detail::dynamic_extent::value, o>>;

}

}
