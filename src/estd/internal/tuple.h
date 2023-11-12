#pragma once

#include "platform.h"
#include "type_traits.h"
#include "fwd/tuple.h"
#include "feature/tuple.h"
#include "utility.h"    // For std::move
#include "tuple/get.h"  // DEBT: Must precede 'visitor.h' likely due to probable lack of 'get' forward declaration
#include "tuple/sparse.h"

// EXPERIMENTAL
#include "variadic/visitor.h"



namespace estd { namespace internal {

// DEBT: 'sparse_tuple' can probably work well in c++03 tuple flavor too

// Needs 'index' to disambiguate from multiple base classes
#if FEATURE_ESTD_IS_EMPTY && FEATURE_ESTD_SPARSE_TUPLE
template <class T, std::size_t index, class enabled = void>
struct sparse_tuple;

template <class T, std::size_t index>
struct sparse_tuple<T, index, typename enable_if<is_empty<T>::value>::type>
{
    static T first() { return T(); }

    typedef T valref_type;
    typedef T const_valref_type;
};


template <class T, std::size_t index>
struct sparse_tuple<T, index, typename enable_if<!is_empty<T>::value>::type>
#else
template <class T, std::size_t index>
struct sparse_tuple
#endif
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


#if __cpp_variadic_templates

template <>
class tuple<>
{
public:
    //static CONSTEXPR int index = 0;
};

template <class T, class ...TArgs>
class tuple<T&, TArgs...> : public tuple<TArgs...>
{
    T& value;

    typedef tuple<TArgs...> base_type;
    using types = variadic::types<T&, TArgs...>;

public:
    typedef T& valref_type;
    typedef const T& const_valref_type;

    constexpr explicit tuple(T& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        value(value)
    {}

    static constexpr int index = sizeof...(TArgs);

    const T& first() const { return value; }

    T& first() { return value; }
};




template <class T, class ...TArgs>
class tuple<T, TArgs...> :
    public tuple<TArgs...>,
    public internal::sparse_tuple<T, sizeof...(TArgs)>
{
    typedef tuple<TArgs...> base_type;
    typedef internal::sparse_tuple<T, sizeof...(TArgs)> storage_type;
    using types = variadic::types<T, TArgs...>;

public:
    template <class UType,
        enable_if_t<is_constructible<T, UType>::value, bool> = true>
    constexpr tuple(UType&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        storage_type(std::forward<UType>(value))
    {}

    constexpr tuple(T&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        storage_type(std::forward<T>(value))
    {}

    using storage_type::first;
    using typename storage_type::valref_type;
    using typename storage_type::const_valref_type;

    explicit tuple() = default;

    static constexpr int index = sizeof...(TArgs);

    typedef T element_type;

    // EXPERIMENTAL, though I think I'm wanting to keep it
    template <class F, class ...Args2>
    bool visit(F&& f, Args2&&...args)
    {
        return types::visitor::visit(internal::visit_tuple_functor{}, *this, f,
            std::forward<Args2>(args)...);
    }

    template <class F, class ...Args2>
    constexpr bool visit(F&& f, Args2&&...args) const
    {
        return types::visitor::visit(internal::visit_tuple_functor{}, *this, f,
            std::forward<Args2>(args)...);
    }
};


#endif

}}
