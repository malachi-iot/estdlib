#pragma once

#include "../../utility.h"
#include "../fwd/functional.h"

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

        model_base(function_type f) : f(f) {}

        model_base(const model_base& copy_from) = default;
        // just like concept_fnptr2, default move constructor somehow
        // results in make_inline2 leaving f uninitialized
        //concept_fnptr1(concept_fnptr1&& move_from) = default;
        model_base(model_base&& move_from) :
            f(move_from.f)
        {}

        // Calls 'exec' down in model, typically
        inline TResult _exec(TArgs&&...args)
        {
            return (this->*f)(std::forward<TArgs>(args)...);
        }
    };


    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        //template <typename U>
        model(F&& u) :
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
};


template <typename TResult, typename... TArgs>
struct function_fnptr2<TResult(TArgs...)>
{
    // this is a slightly less fancy more brute force approach to try to diagose esp32
    // woes
    struct model_base
    {
        typedef TResult (*function_type)(void*, TArgs&&...);

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
};

}}

#endif

}