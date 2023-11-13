#pragma once

#include "get.h"
#include "../feature/tuple.h"

namespace estd { namespace internal {

// DEBT: Consolidate with evaporators
template <bool sparse, class T, std::size_t index, class enabled = void>
struct sparse_tuple;

template <class T, std::size_t index>
struct sparse_tuple<true, T, index, typename enable_if<is_empty<T>::value>::type>
{
    ESTD_CPP_CONSTEXPR_RET static T first() { return T(); }

    typedef T valref_type;
    typedef T const_valref_type;
};


template <bool sparse, class T, std::size_t index>
struct sparse_tuple<sparse, T, index,
    typename enable_if<!is_empty<T>::value || sparse == false>::type>
{
    T value;

    typedef T& valref_type;
    typedef const T& const_valref_type;

    const T& first() const { return value; }

    T& first() { return value; }

#if __cpp_constexpr
    // DEBT: A little sloppy, but should suffice.  This way we make way for converting/direct
    // init constructors without impeding default move constructor
    template <class UType, enable_if_t<!is_same<UType, sparse_tuple>::value, bool> = true>
    explicit constexpr sparse_tuple(UType&& value) :    // NOLINT
        value(std::forward<UType>(value)) {}

    constexpr sparse_tuple(sparse_tuple&&) noexcept = default;
    constexpr sparse_tuple(const sparse_tuple&) = default;
    constexpr sparse_tuple() = default;
#else
    sparse_tuple(const T& value) : value(value) {}
    sparse_tuple() {};
#endif
};

}}