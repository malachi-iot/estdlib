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

template <template <class> class Impl, typename F, typename... Contexts>
struct function_context_provider;

// Adapting from 'context_function'
// DEBT: provider may want to consider an additional level of specialization on
// the impl type.  Right now it's hard wired to fnptr1
template <template <class> class Impl, typename Result, typename... Args, typename... Contexts>
struct function_context_provider<Impl, Result(Args...), Contexts...>
{
protected:
    typedef detail::v2::function<Result(Args...), Impl> function;
    using model_base = typename function::model_base;

public:
    template <class T>
    using function_type = Result (T::*)(Args..., Contexts...);

    template <class T, function_type<T> f>
    struct model : model_base,
        estd::tuple<Contexts...>
    {
        typedef estd::tuple<Contexts...> contexts_type;

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
        model(T* foreign_this, Contexts...contexts) :
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
template <template <class> class Impl, typename F, typename T>
using method_type = typename function_context_provider<Impl, F>::template function_type<T>;

template <template <class> class Impl, typename F, typename T, method_type<Impl, F, T> f>
struct method_model;

template <template <class> class Impl, typename TResult, typename... TArgs, class T,
    method_type<Impl, TResult(TArgs...), T> f>
struct method_model<Impl, TResult(TArgs...), T, f> :
    function_context_provider<Impl, TResult(TArgs...)>::template model<T, f>
{
    typedef typename function_context_provider<Impl, TResult(TArgs...)>::template model<T, f> base_type;

    method_model(T* foreign_this) : base_type(foreign_this) {}
};


}}

#endif

}
