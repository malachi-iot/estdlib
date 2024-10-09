#pragma once

#if __cpp_concepts
#include <concepts>

namespace estd::internal::units {

// DEBT: Should name this something more like Adjuster
template<class T, typename Int = int>
concept Projector = requires(T projector)
{
    typename T::value_type;
    { projector(Int{}) } -> std::same_as<typename T::value_type>;
};

// Deprecated, use 'Projector' instead
template<class T, typename Int = int>
concept Adder = Projector<T, Int>;


}

#endif

