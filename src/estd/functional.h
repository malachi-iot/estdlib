#pragma once

#include "internal/platform.h"

#include "tuple.h"
#include "type_traits.h"

// TODO: Utilize std version of this, if available

namespace estd {

//#if __cplusplus < 201200L
template<class T>
struct less
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template <class T>
struct greater
{
    CONSTEXPR bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs > rhs;
    }
};

/*
#else
template< class T = void >
struct less;
#endif
*/

#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)
namespace internal {


template <class F, class ...TArgs>
struct bind_type
{
    F f;
    tuple<TArgs...> args;

    bind_type(F&& f, TArgs&&...args) :
        args(std::forward<TArgs>(args)...)
    {

    }

    void operator ()()
    {
        f();
    }
};

}

template <class F, class ...TArgs>
auto bind(F&& f, TArgs&&... args) -> internal::bind_type<F, TArgs...>
{
    internal::bind_type<F, TArgs...> b(
                std::move(f),
                std::forward<TArgs...>(args...)
                );

    return b;
}
#endif

}
