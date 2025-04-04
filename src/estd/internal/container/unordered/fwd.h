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

// DEBT: Hard-wired to layer1 stlye

template <class Container, class Traits>
class unordered_map_base;

// DEBT: Will want to live in layer1 namespace
template <unsigned N, class Key, class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key>,
    class Traits = unordered_traits<Key, T, Hash, KeyEqual, Nullable>>
using unordered_map = unordered_map_base<
    uninitialized_array<typename unordered_helper<Traits>::map_control_type, N>,
    Traits>;


}}
