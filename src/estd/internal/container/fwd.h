#pragma once

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace estd {

#if __cpp_concepts
namespace concepts::v2 {

// Loosely conforming to named requirements specified here:
// https://en.cppreference.com/w/cpp/container.html

template <class T>
concept Container = requires(T t)
{
    typename T::value_type;
    typename T::reference;
    typename T::iterator;
    typename T::const_iterator;
    typename T::size_type;

    t.begin();
    t.end();
    t.empty();
};

template <class T>
concept ExtraOnlySequenceContainer = Container<T> && requires(T t)
{
    t.push_back(T{});
};

}

namespace concepts::inline v1
{

template <class T>
concept IndexedContainer = concepts::v2::Container<T> && requires(T t)
{
    t.operator [](0);
};

}
#endif

}