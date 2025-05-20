#pragma once

#include "../../fwd/functional.h"
#include "../../fwd/utility.h"

// DEBT: Use fwd here if we can (maybe not though)
#include "../../array.h"


namespace estd { namespace internal {

template <class T>
struct nullable_traits;

template <class Key,
    class T, class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>, class Nullable = nullable_traits<Key>>
struct unordered_map_traits;

template <class Key, class Hash = hash<Key>, class KeyEqual = equal_to<Key>, class Nullable = nullable_traits<Key>>
struct unordered_set_traits;

template <class Key, class T, class Hash, class KeyEqual, class Nullable>
class unordered_traits;

template <class Container, class Traits>
class unordered_map;

template <class Container, class Traits>
class unordered_set;

}

namespace layer1 {

template <class Key, class T, unsigned N,
    class Hash = hash<Key>,
    class Nullable = internal::nullable_traits<Key>,
    class KeyEqual = equal_to<Key>,
    class Traits = internal::unordered_map_traits<Key, T, Hash, KeyEqual, Nullable>>
using unordered_map = internal::unordered_map<
    internal::uninitialized_array<typename Traits::control_type, N>,
    Traits>;

template <class Key, unsigned N,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>>
using unordered_set = internal::unordered_set<
    internal::uninitialized_array<Key, N>,
    internal::unordered_set_traits<Key, Hash, KeyEqual>>;

}

}

#ifndef ESTD_UNORDERED_MAP_BUCKET_SIZE
#define ESTD_UNORDERED_MAP_BUCKET_SIZE 4
#endif
