#pragma once

namespace estd {

namespace experimental {

// This is a black-boxed array which provides no means by which to access an underlying
// pointer
template <class TImpl>
struct private_array;

}

namespace internal {

namespace impl {

// See reference implementation near bottom of internal/impl/dynamic_array.h
template <class TAllocator, class TPolicy>
struct dynamic_array;


}

}
}