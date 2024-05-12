#pragma once

#include "../../utility.h"
#include "../fwd/functional.h"
#include "functional/fnptr1.h"
#include "functional/fnptr2.h"
#include "functional/virtual.h"
#include <estd/tuple.h>

namespace estd {

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace internal { namespace impl {

template <typename F, typename... Contexts>
struct function_context_provider_base;

template <template <class> class Impl, typename F, typename... Contexts>
struct function_context_provider;

// Adapting from 'context_function'
// DEBT: provider may want to consider an additional level of specialization on
// the impl type.  Right now it's hard wired to fnptr1

template <typename Result, typename... Args, typename... Contexts>
struct function_context_provider_base<Result(Args...), Contexts...>
{
    template <class T>
    using function_type = Result (T::*)(Args..., Contexts...);

    using contexts_type = estd::tuple<Contexts...>;
};

template <template <class> class Impl, typename Result, typename... Args, typename... Contexts>
struct function_context_provider<Impl, Result(Args...), Contexts...> :
    function_context_provider_base<Result(Args...), Contexts...>
{
    using base_type = function_context_provider_base<Result(Args...), Contexts...>;

protected:
    using impl_type = Impl<Result(Args...)>;
    using model_base = typename impl_type::model_base;
    using typename base_type::contexts_type;

public:
    template <class T, typename base_type::template function_type<T> f>
    struct model : model_base,
        contexts_type
    {
        // DEBT: base class needs this public
    public:
        T* const foreign_this;

    protected:
        // EXPERIMENTAL
        model(T* foreign_this, typename model_base::function_type _f) :
            model_base(_f),
            foreign_this{foreign_this}
        {}

    public:
        explicit model(T* foreign_this, Contexts...contexts) :
            model_base(static_cast<typename model_base::function_type>(&model::exec)),
            contexts_type(std::forward<Contexts>(contexts)...),
            foreign_this{foreign_this}
        {

        }

        Result exec(Args...args)
        {
            // FIX: Do apply so that we can get at TContexts... also
            
            return (foreign_this->*f)(std::forward<Args>(args)...);
        }

        Result operator()(Args...args)
        {
            return exec(std::forward<Args>(args)...);
        }
    };

    template <template <typename F> class Provided>
    using provided = Provided<Result(Args...)>;
};

// EXPERIMENTATION
template <template <class> class Impl, typename Result, typename... Args>
struct function_context_provider<
    Impl,
    detail::v2::function<Result(Args...), Impl> > :
    function_context_provider<Impl, Result(Args...)>
{
};

// Almost, but not quite
/*
template <typename F, typename T, typename F2, F2 f>
struct method_model;

template <typename TResult, typename... TArgs, typename T, TResult (T::*f)(TArgs...)>
struct method_model<TResult(TArgs...), T, TResult (T::*)(TArgs...), f> :
    function_context_provider<TResult(TArgs...)>::template model<T, f>
{

}; */
template <typename F, typename T>
using method_type = typename function_context_provider_base<F>::template function_type<T>;

// +++ EXPERIMENTAL
template <typename R, typename... Args>
struct method_type_helper
{
    // DEBT: monostate is dummy as we test things
    using type = method_type<R(Args...), estd::monostate>;
};

struct method_model_helper2
{
    //typedef TResult (model_base::*function_type)(TArgs...);

    template <class T, class R, typename... Args>
    static void do_something(T* t, R(T::*v)(Args...))
    {
        using type = method_type<R(Args...), T>;
        type test1(v);
    }
};
// ---

template <typename F, typename T, method_type<F, T> f,
    template <class> class Impl = detail::impl::function_fnptr1>
struct method_model;

template <typename Result, typename... Args, class T,
    method_type<Result(Args...), T> f, template <class> class Impl>
struct method_model<Result(Args...), T, f, Impl> :
    function_context_provider<Impl, Result(Args...)>::template model<T, f>
{
    using base_type = typename function_context_provider<Impl, Result(Args...)>::template model<T, f>;

    explicit method_model(T* foreign_this) : base_type(foreign_this) {}
};


}}

#endif

}
