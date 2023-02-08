#pragma once

#include "../../utility.h"
#include "../fwd/functional.h"
#include <estd/tuple.h>

#include <string.h> // for memcpy

namespace estd {

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)

namespace detail { namespace impl {

// Since we prefer 'fnptr1', but can't discount the value of
// 'virtual', formalizing different techniques into impls here
// DEBT: May need to wrap these in a provider so that TResult(TArgs...) doesn't interfere too
// much with us switching to different impls

// We like this way best, but due to [1] it may not be viable.
// However, they say that:
// "Deleting a polymorphic object without a virtual destructor
//  is permitted if the object is referenced by a pointer to its
//  class, rather than via a pointer to a class it inherits from."
// This might be possible with a manual function pointer deleter.
template <typename TResult, typename... TArgs>
struct function_fnptr1<TResult(TArgs...)>
{
    // function pointer approach works, but if we have to add in a virtual destructor
    // (which is likely) then we are faced with incurring that overhead anyway so might
    // switch over to virtual operator() in that case
    struct model_base
    {
        typedef TResult (model_base::*function_type)(TArgs...);
        typedef void (*deleter_type)(model_base*);  // EXPERIMENTAL

        const function_type f;

        constexpr model_base(function_type f) : f(f) {}

        model_base(const model_base& copy_from) = default;
        // just like concept_fnptr2, default move constructor somehow
        // results in make_inline2 leaving f uninitialized
        //concept_fnptr1(concept_fnptr1&& move_from) = default;
        constexpr model_base(model_base&& move_from) :
            f(move_from.f)
        {}

        // Calls 'exec' down in model, typically
        inline TResult _exec(TArgs&&...args)
        {
            return (this->*f)(std::forward<TArgs>(args)...);
        }
    };


    // Inline flavor
    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        //template <typename U>
        constexpr model(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        TResult exec(TArgs...args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };


    // Inline also, for when empty closure is present
    template <typename F>
    struct sparse_model : model_base
    {
        typedef model_base base_type;

        constexpr sparse_model(F&&) :
            base_type(static_cast<typename base_type::function_type>(&sparse_model::exec))
        {
            static_assert(estd::is_empty<F>::value, "Must only operate on empty closures");
        }

        // NOTE: Scary stuff, but our constructor ensures careful usage of this is safe
        constexpr static F& f()
        {
            return *(F*) nullptr;
        }

        TResult exec(TArgs...args)
        {
            return f()(std::forward<TArgs>(args)...);
        }
    };

    // Inline flavor which can be copied around without foreknowledge of 'F'
    // Not merged with regular model because this one is less safe so unless
    // this level of copyability is really needed, use 'model'
    template <int sz = 0>
    struct copyable_model : model_base
    {
        typedef model_base base_type;

        // EXPERIMENTAL - if we like this, spin off a psueod model
        // without the regular function pointer at its base, just for
        // limited scenarios where we'd prefer only one fnptr here
        // instead of 2
        union util_param
        {
            estd::tuple<TArgs...>* params;
            const estd::byte* copy_from;
        };

        //void (*util)(int mode, util_param);

        estd::byte unsafe_closure[sz];

        // Copy may be safe, but underlying unsafe_closure still sort of isn't!
        template <class F>
        static void copy_by_constructor(const estd::byte* from, estd::byte* to)
        {
            auto f = (F*) from;

            new (to) F(*f);
        }

        // Specifically copies the unsafe_closure portion via F copy constructor
        void (* const copy)(const estd::byte* from, estd::byte* to);

        template <class F>
        static TResult helper(estd::byte* unsafe_closure, TArgs...args)
        {
            auto& f = *(F*)unsafe_closure;

            return f(std::forward<TArgs>(args)...);
        }

        // EXPERIMENTAL
        template <class F>
        void util_impl(int mode, util_param p)
        {
            switch(mode)
            {
                case 0:
                {
                    auto f = (F*)unsafe_closure;

                    // FIX: How do we invoke
                    // 'f' with a tuple?
                    break;
                }

                case 1:
                    copy_by_constructor<F>(p.copy_from);
                    break;

                default:
                    break;
            }
        }

        // NOTE: This relies on an external party placing 'copyable_model' in a memory
        // space big enough to hold all of 'f'
        template <class F,
            estd::enable_if_t<!estd::is_base_of<model_base, F>::value, bool> = true>
        copyable_model(const F& f) :
            base_type(static_cast<typename base_type::function_type>(&copyable_model::exec_<F>)),
            copy{copy_by_constructor<F>}
        {
            static_assert((sz + estd::is_empty<F>::value) >= sizeof(f),
                "sz MUST be greater than sizeof(F) and SHOULD match");
            new (unsafe_closure) F(f);
        }

        // DEBT: I find it odd that rvalue is always favored , even when an lvalue is supplied,
        // unless incoming F is itself a const.  Perhaps if it detects no difference between a copy
        // and a move, it implies a move?
        template <class F, class F2 = estd::remove_reference_t<F>,
            estd::enable_if_t<!estd::is_base_of<model_base, F2>::value, bool> = true>
        copyable_model(F&& f) :
            base_type(static_cast<typename base_type::function_type>(&copyable_model::exec_<F2>)),
            copy{copy_by_constructor<F2>}
        {
            static_assert((sz + estd::is_empty<F2>::value) >= sizeof(f),
                "sz MUST be greater than sizeof(F) and SHOULD match");
            new (unsafe_closure) F2(std::move(f));
        }

        template <class F>
        copyable_model(const model<F>& copy_from) : copyable_model(copy_from.f)
        {

        }

        // EXPERIMENTAL
        template <class F>
        copyable_model(const sparse_model<F>& copy_from) : copyable_model(copy_from.f())
        {

        }


        template <int sz__>
        copyable_model(const copyable_model<sz__>& copy_from) :
            base_type(copy_from.f),
            copy(copy_from.copy)
        {
            // DEBT: Doing this because we would rather re-copy 'f' as pointer types
            // are trivial rather than dance around possible padding issues.  Specifically,
            // identifying correct sz_ for JUST unsafe_closure involves those issues. That said,
            // doing that dance would be a good optimization - that, or place a
            // 'unsafe_closure[]' right into model_base
            //memcpy(this, copy_from, sz_);

            // Since assuming POD/trivial appears to be dangerous sometimes,
            // doing proper copy via fnptr
            copy(copy_from.unsafe_closure, unsafe_closure);
        }

        template <class F>
        TResult exec_(TArgs&&...args)
        {
            auto f = (F*)unsafe_closure;

            return (*f)(std::forward<TArgs>(args)...);
        }
    };

    template <class F, int sz = sizeof(F) - estd::is_empty<F>::value>
    static inline auto make_copyable(F f) -> copyable_model<sz>
    {
        return copyable_model<sz>(f);
    }
};

// BEWARE
// https://stackoverflow.com/questions/29188190/function-pointer-memory-usage implies
// this approach may not work directly as is
template <typename TResult, typename... TArgs>
struct function_fnptr2<TResult(TArgs...)>
{
    // this is a slightly less fancy more brute force approach to try to diagose esp32
    // woes
    struct model_base
    {
        typedef TResult (*function_type)(void*, TArgs...);

        const function_type _f;

        model_base(function_type f) : _f(f) {}

        model_base(const model_base& copy_from) = default;
        // DEBT: For some reason ESP32's default move constructor
        // doesn't initialize _f
        //concept_fnptr2(concept_fnptr2&& move_from) = default;
        model_base(model_base&& move_from) :
            _f(std::move(move_from._f))
        {}

        inline TResult _exec(TArgs&&...args)
        {
            return _f(this, std::forward<TArgs>(args)...);
        }
    };

    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        model(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model::__exec)),
            f(std::forward<F>(u))
        {
        }

        /*
        model_fnptr2(const model_fnptr2& copy_from) = default;
        //model_fnptr2(model_fnptr2&& move_from) = default;
        model_fnptr2(model_fnptr2&& move_from) :
            base_type(std::move(move_from)),
            f(std::move(move_from.f))
        {} */

        F f;

        // TODO: Consolidate different models down to a model_base since they
        // all need this exec function
        TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }

        static TResult __exec(void* _this, TArgs&&...args)
        {
            auto __this = ((model*)_this);

            return __this->f(std::forward<TArgs>(args)...);
        }
    };

    // DEBT: This alias isn't a sparse model, just saving this optimization
    // for another day
    template <class F>
    using sparse_model = model<F>;
};


template <typename TResult, typename... TArgs>
struct function_virtual<TResult(TArgs...)>
{
    struct model_base
    {
        virtual TResult _exec(TArgs...args) = 0;
        virtual ~model_base() = default;
    };

    template <class F>
    struct model : model_base
    {
        model(F&& u) :
            f(std::forward<F>(u))
        {
        }

        F f;

        virtual TResult _exec(TArgs...args) override
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    // DEBT: This alias isn't a sparse model, just saving this optimization
    // for another day
    template <class F>
    using sparse_model = model<F>;
};

}}

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
    typedef detail::function<TResult(TArgs...)> function;
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
