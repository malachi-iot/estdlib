#pragma once

#include "fwd.h"

#include "../iterator_standalone.h"
#include "../raw/memory.h"
#include "../raw/type_traits.h"

namespace estd {

namespace internal {

// Move flavor
template <class Out, class In>
constexpr Out* move_or_copy(void* dest, In& src, true_type)
{
    return ::new (dest) Out(std::move(src));
}

// Copy flavor
template <class Out, class In>
constexpr Out* move_or_copy(void* dest, const In& src, false_type)
{
    return ::new (dest) Out(src);
}

}

// Adapted from https://en.cppreference.com/cpp/memory/uninitialized_move
template<class InputIt, class ForwardIt>
ESTD_CPP_CONSTEXPR(14) ForwardIt uninitialized_move(InputIt first, InputIt last,
    ForwardIt d_first)
{
    using is_lvalue_ref = is_lvalue_reference<decltype(*first)>;
    using ValueType = typename iterator_traits<ForwardIt>::value_type;
    ForwardIt current = d_first;
    for (; first != last; ++first, ++current)
    {
        void* addr = addressof(*current);
        internal::move_or_copy<ValueType>(addr, *first, is_lvalue_ref{});
        /*
        if constexpr (is_lvalue_reference<decltype(*first)>::value)
            ::new (addr) ValueType(std::move(*first));
        else
            ::new (addr) ValueType(*first); */
    }
    return current;
}

}