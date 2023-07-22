#pragma once

namespace estd {

namespace experimental {

// This is a black-boxed array which provides no means by which to access an underlying
// pointer
template <class TImpl>
struct private_array;

}

namespace internal {

template <class Allocator, class Policy>
class basic_string;

}

}