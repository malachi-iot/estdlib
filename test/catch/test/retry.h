#pragma once

#include <estd/internal/priority_queue.h>
#include <estd/internal/container/unordered/fwd.h>

#include "nontrivial.h"

namespace estd { namespace test {

template <class Key, class T, size_t N = 10>
class retry_tracker
{
public:
    using map_type = internal::unordered_map<N, Key, T>;
    using value_type = typename map_type::value_type;

private:
    internal::unordered_map<N, Key, T> tracked_;
    layer1::priority_queue<value_type*, N> queue_;

public:
    void track(Key key, const T& value)
    {
        tracked_.emplace(key, value);
    }
};

}}
