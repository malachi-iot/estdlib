#pragma once

// In response to https://github.com/malachi-iot/estdlib/issues/11

namespace estd { namespace internal { namespace list {

struct null_iterator {};

template <class Node, class Data = Node>
struct intrusive_traits;

}}}
