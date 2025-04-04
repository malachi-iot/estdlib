#pragma once

#include "../../fwd/functional.h"
#include "../../fwd/utility.h"

namespace estd { namespace internal {

template <class T>
struct nullable_traits;

template <class Key, class T, class Hash, class KeyEqual, class Nullable = nullable_traits<Key>>
class unordered_traits;

// DEBT: Hard-wired to layer1 stlye

template <unsigned sz, class Traits>
class unordered_map_base;

// DEBT: Will want to live in layer1 namespace
template <unsigned N, class Key, class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key>>
using unordered_map = unordered_map_base<N, unordered_traits<Key, T, Hash, KeyEqual, Nullable>>;


}}
