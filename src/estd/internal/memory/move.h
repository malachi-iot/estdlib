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

#if FEATURE_ESTD_STD_MEMORY_ALIAS && __cplusplus >= 201703L
using std::uninitialized_move;
using std::uninitialized_move_n;
#else
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

template<class InputIt, class Size, class ForwardIt>
ESTD_CPP_CONSTEXPR(14) pair<InputIt, ForwardIt> uninitialized_move_n(InputIt first, Size count,
    ForwardIt d_first)
{
    // 03JUN16 MB DEBT: We can optimize this - if we detect InputIt is a random-access
    // iterator (often the case) we can cascade out to regular uninitialized_move
    using is_lvalue_ref = is_lvalue_reference<decltype(*first)>;
    using ValueType = typename iterator_traits<ForwardIt>::value_type;
    ForwardIt current = d_first;
    for (; count > 0; ++current, ++first, --count)
    {
        void* addr = addressof(*current);
        internal::move_or_copy<ValueType>(addr, *first, is_lvalue_ref{});
    }
    return { first, current };
}
#endif

}