#pragma once

#include "../../fwd/functional.h"
#include "../../fwd/utility.h"

namespace estd { namespace internal {

template <class T>
struct nullable_traits;

// DEBT: Hard-wired to layer1 stlye

template <unsigned sz,
    class Key,
    class T,
    class Hash = hash<Key>,
    class Nullable = nullable_traits<Key>,
    class KeyEqual = equal_to<Key> >
class unordered_map;


}}
