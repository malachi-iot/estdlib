#pragma once

#if __cpp_concepts
#include <concepts>

namespace estd::internal::units {

// DEBT: Should name this something more like Adjuster
template<class T, typename TInt = int>
concept Adder = requires(T adder)
{
    typename T::value_type;
    { adder(TInt{}) } -> std::same_as<typename T::value_type>;
};


}

#endif

