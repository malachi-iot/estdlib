#pragma once

#include "../internal/platform.h"
#include "../internal/functional.h"
#include "../internal/value_evaporator.h"

namespace estd { namespace experimental {

// There's some debate whether std::function constructor does or is even allowed dynamic
// allocation.  It still seems like it's a must.
// https://stackoverflow.com/questions/18453145/how-is-stdfunction-implemented

template <typename F>
class static_function;

#ifdef __cpp_variadic_templates
template <typename TResult, typename... TArgs>
class static_function<TResult(TArgs...)> :
    public detail::function<TResult(TArgs...)>
{
    typedef detail::function<TResult(TArgs...)> base_type;

public:
    template <typename F>
    static_function(F&& f)
    {
        static typename base_type::template model<F> m(std::move(f));

        base_type::m = &m;
    }
};
#endif


}}
