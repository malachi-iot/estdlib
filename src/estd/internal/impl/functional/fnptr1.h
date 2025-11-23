#pragma once

#include "fwd.h"

// 23NOV25 MB DEBT: No dtor/utility assist here.  Would be a FIX but
// so far we far prefer fnptr2 and virtual

// Arduino compatibility
#pragma push_macro("F")
#undef F

namespace estd { namespace detail { namespace impl {

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
template <typename Result, typename... Args>
struct function_fnptr1<Result(Args...)>
{
    // function pointer approach works, but if we have to add in a virtual destructor
    // (which is likely) then we are faced with incurring that overhead anyway so might
    // switch over to virtual operator() in that case
    struct model_base
    {
        typedef Result (model_base::*function_type)(Args...);
        typedef void (*deleter_type)(model_base*);  // EXPERIMENTAL
        typedef void (model_base::*deleter2_type)();  // EXPERIMENTAL

        const function_type f;

        constexpr explicit model_base(function_type f) : f(f) {}

        // Calls 'exec' down in model, typically
        inline Result operator()(Args&&...args)
        {
            return (this->*f)(std::forward<Args>(args)...);
        }
    };


    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        //template <typename U>
        constexpr explicit model(F&& u) :
            base_type(
                // casts Result (model<F>::*)(Args...) -> Result (model_base::*)(Args...)
                static_cast<typename base_type::function_type>(&model::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        // DEBT: Use rvalue here
        Result exec(Args...args)
        {
            return f(std::forward<Args>(args)...);
        }
    };

    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            model_base(static_cast<typename model_base::function_type>(&method_model::exec)),
            object_{t}
        {}

        T* const object_;
        // DEBT: Use rvalue here
        Result exec(Args...args)
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }
    };
};


// Special version which calls dtor right after function invocation
template <typename Result, typename... Args>
struct function_fnptr1_opt<Result(Args...)>
{
    struct model_base
    {
        typedef Result (model_base::*function_type)(Args...);

        const function_type f;

        constexpr explicit model_base(function_type f) : f(f) {}
    };

    template <typename F>
    struct model : model_base
    {
        typedef model_base base_type;

        //template <typename U>
        constexpr explicit model(F&& u) :
            base_type(
                static_cast<typename base_type::function_type>(&model::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        Result exec(Args...args)
        {
            Result r = f(std::forward<Args>(args)...);
            f.~F();
            return r;
        }
    };
};

}}}

#pragma pop_macro("F")

