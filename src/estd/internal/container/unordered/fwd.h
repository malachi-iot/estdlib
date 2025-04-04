#pragma once

#include "../../fwd/functional.h"
#include "../../fwd/utility.h"

// DEBT: Use fwd here if we can (maybe not though)
#include "../../array.h"


namespace estd { namespace internal {

template <class Traits>
struct unordered_helper;

template <class T>
struct nullable_traits;

template <class Key, class T, class Hash, class KeyEqual, class Nullable = nullable_traits<Key>>
class unordered_traits;

template <class Container, class Traits>
class unordered_map_base;

template <class Container, class Traits>
class unordered_set_base;

// DEBT: Will want to live in layer1 namespace
template <unsigned N, class Key, class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key>,
    class Traits = unordered_traits<Key, T, Hash, KeyEqual, Nullable>>
using unordered_map = unordered_map_base<
    uninitialized_array<typename unordered_helper<Traits>::map_control_type, N>,
    Traits>;


template <class Container,
    class Key = typename Container::value_type,
    class Hash = hash<Key>,
    class KeyEqual = equal_to<Key>>
using unordered_set = unordered_set_base<Container, unordered_traits<Key, Key, Hash, KeyEqual>>;

}}
