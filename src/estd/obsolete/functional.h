#pragma once

#include "../internal/platform.h"
#include "../type_traits.h"
#include "../tuple.h"

#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)

namespace estd { namespace obsolete {

template <class F>
struct function
{
    F f;

    function() NOEXCEPT = default;

    template <class F2>
    function(F2 f) : f(f) {}

    // deviates from spec here, but doing this to conform with how bind likes to
    // do things
    function(F&& f) : f(std::move(f)) {}

    // deviates from spec, function class itself is supposed to take these args
    template <class ...TArgs>
    auto operator()(TArgs&&...args) -> decltype (f(args...))
    {
        return f(std::forward<TArgs>(args)...);
    }

    // UNTESTED
    function& operator=(F&& f)
    {
        this->f = std::move(f);
        return *this;
    }

    function& operator=(reference_wrapper<F> f) NOEXCEPT
    {
        this->f = f;
        return *this;
    }
};

namespace internal {


template <class F, class ...TArgs>
struct bind_type : function<F>
{
    typedef function<F> base_type;

    tuple<TArgs...> args;

    bind_type(F&& f, TArgs&&...args) :
        base_type(std::move(f)),
        args(std::forward<TArgs>(args)...)
    {

    }

    auto operator ()() -> decltype (apply(std::move(base_type::f), std::move(args)))
    {
        return apply(std::move(base_type::f), std::move(args));
    }
};

}

template <class F, class ...TArgs>
auto bind(F&& f, TArgs&&... args) -> internal::bind_type<F, TArgs...>
{
    internal::bind_type<F, TArgs...> b(
                std::move(f),
                std::forward<TArgs>(args)...
                );

    return b;
}

}}

#endif
