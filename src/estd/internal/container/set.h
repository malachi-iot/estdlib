#pragma once

#include "../fwd/functional.h"

namespace estd { namespace internal {

template <class Container,
    class Key = typename Container::value_type,
    class Compare = less<Key> >
class set;

}}
