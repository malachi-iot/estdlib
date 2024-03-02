#pragma once

#include "../platform.h"

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace estd {

#if __cpp_concepts
namespace concepts { inline namespace v1 {

namespace impl {

template <class T>
concept Container =
requires(T c)
{
    typename T::value_type;
    typename T::size_type;
};

}

// Maps to https://en.cppreference.com/w/cpp/named_req/Container
template <class T>
concept Container = impl::Container<T> &&
requires(T c)
{
    typename T::iterator;
    typename T::pointer;
    //typename T::allocator_type;

    c.begin();
    c.end();
    c.empty();
    c.max_size();
};

}}
#endif

namespace experimental {

// This is a black-boxed array which provides no means by which to access an underlying
// pointer
template <class TImpl>
struct private_array;

}

namespace internal {

#if __cpp_concepts
template <class T>
concept AllocatedArrayImpl = requires(T a)
{
    typename T::size_type;
    typename T::allocator_type;
    typename T::allocator_traits;
    //typename T::policy_type;

    a.get_allocator();
    a.size();
};

template <class T>
concept BufferPolicy = requires
{
    typename T::size_type;
    T::is_constant();
};


template <class T>
concept AllocatedArrayOperations = requires
{
    T::copy_into();
};
#endif

template <ESTD_CPP_CONCEPT(AllocatedArrayImpl) TImpl>
class allocated_array;

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
