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

#if __cpp_variadic_templates

template <bool sparse>
class tuple<sparse>
{
public:
    //static CONSTEXPR int index = 0;
};


template <bool sparse, class T, class ...Args>
class tuple<sparse, T, Args...> :
    public tuple<sparse, Args...>,
    public internal::sparse_tuple<sparse, T, sizeof...(Args)>
{
    typedef tuple<sparse, Args...> base_type;
    typedef internal::sparse_tuple<sparse, T, sizeof...(Args)> storage_type;
    using types = variadic::types<T, Args...>;

public:
    template <class UType,
        enable_if_t<is_constructible<T, UType>::value, bool> = true>
    constexpr explicit tuple(UType&& value, Args&&...args) :
        base_type(std::forward<Args>(args)...),
        storage_type(std::forward<UType>(value))
    {}

    constexpr explicit tuple(T&& value, Args&&...args) :
        base_type(std::forward<Args>(args)...),
        storage_type(std::forward<T>(value))
    {}

    using storage_type::first;
    using typename storage_type::valref_type;
    using typename storage_type::const_valref_type;

    explicit tuple() = default;

    //static constexpr int index = sizeof...(Args);

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
