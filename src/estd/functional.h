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
        f(std::move(f)),
        args(std::forward<TArgs>(args)...)
    {

    }

    // index_sequence parameter makes it easier to get at
    // size_t... Is, which in turn lets us do the get<Is>... call
    // TODO: refactor make+use 'apply'
    // https://en.cppreference.com/w/cpp/utility/apply
    template <class Tuple, size_t... Is>
    void invoker(Tuple t, index_sequence<Is...>)
    {
        return f(get<Is>(t)...);
    }

    void operator ()()
    {
        apply(std::move(f), std::move(args));
        /*
        typedef make_index_sequence<
                    tuple_size<
                        tuple<TArgs...>
                    >::value
                > seq;

        invoker(args, seq {}); */

        //f(get<>(args)...);
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
