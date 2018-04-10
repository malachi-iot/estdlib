#pragma once

#include "utility.h"
#include "functional.h"
#include "array.h"

namespace estd {

template <class Key, class T, class Compare = estd::less<Key> >
class map_base
{

};

namespace layer1 {

template <class Key, class T, std::size_t size, class Compare = estd::less<Key> >
class map
{
    array<pair<Key, T>, size> _map;
};

}

}
