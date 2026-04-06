#pragma once

#include "../../../new.h"
#include "../../rtto.h"
#include "../../value_evaporator.h"

#include "fwd.h"

namespace estd { namespace detail { namespace impl {

// DEBT: Move/consolidate this up to value_evaporator.... maybe
template <class F, bool is_empty = estd::is_empty<F>()>
class functor_provider
{
protected:
    F functor_;

public:
    F& functor() { return functor_; }

    functor_provider() = delete;

    constexpr explicit functor_provider(F&& f) :
        functor_(std::move(f))
    {}

    constexpr explicit functor_provider(const F& f) :
        functor_(f)
    {}
};

template <class F>
class functor_provider<F, true>
{
#if FEATURE_STD_TYPE_TRAITS
    static_assert(std::is_trivially_copyable<F>::value, "F must be trivially copyable");
#endif

public:
    // F is trivially copyable and an empty struct.  That means no copy really occurs
    constexpr F functor() { return *(F*)this; }  // Would prefer nullptr, but compiler warnings get aggressive

    functor_provider() = delete;
    constexpr explicit functor_provider(F&&) {}
    constexpr explicit functor_provider(const F&) {}
};


template <typename Result, typename... Args, fn_options o>
struct function_virtual<Result(Args...), o> : internal::rtto_base
{
    static constexpr fn_options options = o;

#if FEATURE_ESTD_FUNCTION_RVALUE
    using function_type = Result (*)(void*, Args&&...);
#else
    using function_type = Result (*)(void*, Args...);
#endif

    struct model_base : rtto_base::virtual_base
    {
#if FEATURE_ESTD_FUNCTION_RVALUE
        virtual Result operator()(Args&&...args) = 0;
#else
        virtual Result operator()(Args...args) = 0;
#endif
        //virtual ~model_base() = default;
#if FEATURE_ESTD_GH135
#endif
    };

    template <class F>
    struct model : model_base, functor_provider<F>
    {
        using rtto = internal::rtto<model>;
        using provider = functor_provider<F>;

        constexpr explicit model(const F& u) :
            provider(u)
        {}

        constexpr explicit model(F&& u) :
            provider(std::forward<F>(u))
        {
        }

        ESTD_CPP_DEFAULT_RULE_OF_5(model)

#if FEATURE_ESTD_FUNCTION_RVALUE
        Result operator()(Args&&...args) override
#else
        Result operator()(Args...args) override
#endif
        {
            return provider::functor()(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        int copy_to(void* dest, int sz) const override
        {
            return rtto::copy(this, dest, sz);
        }

        int move_to(void* dest, int sz) override
        {
            return rtto::move(this, dest, sz);
        }

#if FEATURE_ESTD_RTTO_GET_METADATA
        const metadata* get_metadata() const override
        {
            return model_base::template get<model>();
        }
#endif
#endif
    };


    // 13MAY24 MB EXPERIMENTAL replacement for 'thisify'
    template <class T, Result (T::*f)(Args...)>
    struct method_model : model_base
    {
        constexpr explicit method_model(T* t) :
            object_{t}
        {}

        T* const object_;

#if FEATURE_ESTD_FUNCTION_RVALUE
        Result operator()(Args&&...args) override
#else
        Result operator()(Args...args) override
#endif
        {
            return (object_->*f)(std::forward<Args>(args)...);
        }

#if FEATURE_ESTD_GH135
        int copy_to(void* dest, int sz) const override
        {
            return ENOSYS;
        }

        int move_to(void* dest, int sz) override
        {
            return ENOSYS;
        }

#if FEATURE_ESTD_RTTO_GET_METADATA
        const metadata* get_metadata() const override
        {
            // 50% implemented
            return model_base::template get<method_model>();
        }
#endif
#endif
    };
};


}}}
