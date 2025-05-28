#pragma once

#include "../../../span.h"
#include "../../fwd/functional.h"
#include "../../fwd/utility.h"

// DEBT: Use fwd here if we can (maybe not though)
#include "../../array.h"

#include "features.h"

#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
#define ESTD_UNORDERED_MAP_NULLABLE_OPT , Nullable
#else
#define ESTD_UNORDERED_MAP_NULLABLE_OPT
#endif

namespace estd { namespace internal {

template <class T>
struct nullable_traits;

template <class Key, class Mapped>
struct unordered_map_traits_control;

template <class Key,
    class T, class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>
#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
    , class Nullable = nullable_traits<Key>
#endif
    >
struct unordered_map_traits;

template <class Key, class Hash = hash<Key>, class KeyEqual = equal_to<Key>
#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
    , class Nullable = nullable_traits<Key>
#endif
    >
struct unordered_set_traits;

template <class Key, class T, class Hash, class KeyEqual
#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
    , class Nullable
#endif
    >
class unordered_traits;

template <class Container, class Traits>
class unordered_map;

template <class Container, class Traits>
class unordered_set;

}

namespace layer1 {

template <class Key, class T, unsigned N,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>,
#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
    class Nullable = internal::nullable_traits<Key>,
#endif
    class Traits = internal::unordered_map_traits<Key, T, Hash, KeyEqual ESTD_UNORDERED_MAP_NULLABLE_OPT>>
using unordered_map = internal::unordered_map<
    internal::uninitialized_array<typename Traits::control_type, N>,
    Traits>;

template <class Key, unsigned N,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>,
    class Traits = internal::unordered_set_traits<Key, Hash, KeyEqual>>
using unordered_set = internal::unordered_set<
    internal::uninitialized_array<Key, N>,
    Traits>;

}

namespace layer2 {

template <class Key, class T, unsigned N,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>,
#if ESTD_UNORDERED_MAP_EXPLICIT_NULLABLE
    class Nullable = internal::nullable_traits<Key>,
#endif
    class Traits = internal::unordered_map_traits<Key, T, Hash, KeyEqual ESTD_UNORDERED_MAP_NULLABLE_OPT>>
using unordered_map = internal::unordered_map<
    estd::span<typename Traits::control_type, N>,
    Traits>;

// UNTESTED
template <class Key, unsigned N,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>>
using unordered_set = internal::unordered_set<
    estd::span<Key, N>,
    internal::unordered_set_traits<Key, Hash, KeyEqual>>;

}

}
