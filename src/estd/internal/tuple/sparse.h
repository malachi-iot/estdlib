#pragma once

#include "get.h"
#include "../feature/tuple.h"

namespace estd { namespace internal {

// DEBT: Consolidate with evaporators

// Needs 'index' to disambiguate from multiple base classes

template <class T, size_t index>
class tuple_storage<true, T, index, typename enable_if<is_empty<T>::value>::type>
{
public:
    tuple_storage() = default;
    // DEBT: make_tuple needs this guy because chain isn't smart enough to exclude the placeholder type
    // while recusring up the ctor chain.  Would be better to do the exclusion, but this will do
    constexpr tuple_storage(T&&) {}

    ESTD_CPP_CONSTEXPR_RET static T first() { return T(); }

    typedef T valref_type;
    typedef T const_valref_type;
};


template <bool sparse, class T, size_t index>
class tuple_storage<sparse, T, index,
    typename enable_if<!is_empty<T>::value || sparse == false>::type>
{
    T value;

public:
    typedef T& valref_type;
    typedef const T& const_valref_type;

    constexpr const T& first() const { return value; }

    T& first() { return value; }

#if __cpp_constexpr
    // DEBT: A little sloppy, but should suffice.  This way we make way for converting/direct
    // init constructors without impeding default move constructor
    template <class UType, enable_if_t<!is_same<UType, tuple_storage>::value, bool> = true>
    explicit constexpr tuple_storage(UType&& value) :    // NOLINT
        value(std::forward<UType>(value)) {}

    constexpr tuple_storage(tuple_storage&&) noexcept = default;
    constexpr tuple_storage(const tuple_storage&) = default;
    constexpr tuple_storage() = default;
#else
    tuple_storage(const T& value) : value(value) {}
    tuple_storage() {};
#endif
};

}}
