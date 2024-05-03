#pragma once

#include "../../utility.h"
#include "../fwd/functional.h"
#include "functional/fnptr1.h"
#include "functional/fnptr2.h"
#include "functional/virtual.h"
#include <estd/tuple.h>

namespace estd {

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)

namespace internal { namespace impl {

template <typename F, typename... TContexts>
struct function_context_provider;

// Adapting from 'context_function'
// DEBT: provider may want to consider an additional level of specialization on
// the impl type.  Right now it's hard wired to fnptr1
template <typename TResult, typename... TArgs, typename... TContexts>
struct function_context_provider<TResult(TArgs...), TContexts...>
{
protected:
    typedef detail::function<TResult(TArgs...),
        estd::detail::impl::function_fnptr1<TResult(TArgs...)> > function;
    using model_base = typename function::model_base;

public:
    template <class T>
    using function_type = TResult (T::*)(TArgs..., TContexts...);

    template <class T, function_type<T> f>
    struct model : model_base,
        estd::tuple<TContexts...>
    {
        typedef estd::tuple<TContexts...> contexts_type;

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
        model(T* foreign_this, TContexts...contexts) :
            model_base(static_cast<typename model_base::function_type>(&model::exec)),
            contexts_type(std::forward<TContexts>(contexts)...),
            foreign_this{foreign_this}
        {

        }

        TResult exec(TArgs...args)
        {
            // FIX: Do apply so that we can get at TContexts... also
            
            return (foreign_this->*f)(std::forward<TArgs>(args)...);
        }
    };

    template <template <typename F> class TProvided>
    using provided = TProvided<TResult(TArgs...)>;
};

// EXPERIMENTATION
template <typename TResult, typename... TArgs>
struct function_context_provider<detail::function<TResult(TArgs...)> > :
    function_context_provider<TResult(TArgs...)>
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
using method_type = typename function_context_provider<F>::template function_type<T>;

template <typename F, typename T, method_type<F, T> f>
struct method_model;

template <typename TResult, typename... TArgs, class T, method_type<TResult(TArgs...), T> f>
struct method_model<TResult(TArgs...), T, f> :
    function_context_provider<TResult(TArgs...)>::template model<T, f>
{
    typedef typename function_context_provider<TResult(TArgs...)>::template model<T, f> base_type;

    method_model(T* foreign_this) : base_type(foreign_this) {}
};


}}

#endif

}
