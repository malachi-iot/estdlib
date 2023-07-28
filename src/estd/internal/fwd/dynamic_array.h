#pragma once

namespace estd {

namespace experimental {

// This is a black-boxed array which provides no means by which to access an underlying
// pointer
template <class TImpl>
struct private_array;

}

namespace internal {

template <class TImpl>
class allocated_array;;

namespace impl {

// See reference implementation near bottom of internal/impl/dynamic_array.h
template <class TAllocator, class TPolicy>
struct dynamic_array;

// TODO: Fixup name.  Specializer to reveal size of either
// an explicitly-sized or null-terminated entity
template <class TAllocator, bool null_terminated, bool size_equals_capacity>
struct dynamic_array_length;


}

template <class TImpl>
class dynamic_array;

}
}
