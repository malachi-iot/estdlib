/**
 * @file
 *
 * flavor of functional with minimized dependencies
 */
#pragma once

#include "fwd/functional.h"
#include "impl/functional.h"
#include "functional/hash.h"
#include "../type_traits.h"
#include "../tuple.h"

namespace estd {

template <class T>
struct less
{
    ESTD_CPP_CONSTEXPR_RET bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template <class T>
struct greater
{
    ESTD_CPP_CONSTEXPR_RET bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs > rhs;
    }
};

template <class T>
struct equal_to
{
    ESTD_CPP_CONSTEXPR_RET bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs == rhs;
    }
};

namespace experimental {

#ifdef FEATURE_CPP_ALIGN
// kind of a blend of reference_wrapper and aligned_storage
// experimental, but very useful (abusing raw_instance_provider for this right now)
// NOTE: This in theory could be 1:1 with a fixed allocator, who should be using aligned_storage too
// DEBT: Rework all this with new 'variant_storage' behavior in mind
template <class T, size_t Align = alignof(T)>
class instance_wrapper
{
    // Not sure an item all by its lonesome in a class is gonna benefit from aligned_storage
    // perhaps instead we should make it a parent?
    typename estd::aligned_storage<sizeof(T), Align>::type _value;

public:
    typedef T value_type;

    value_type& value() { return *(T*)&_value; }
    const value_type& value() const { return *(T*)&_value; }

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
    value_type& construct(TArgs&&...args)
    {
        void* loc = reinterpret_cast<void*>(&_value);
        new (loc) value_type(std::forward<TArgs>(args)...);
        return value();
    }
#else
    value_type& construct()
    {
        new (&_value) value_type();
        return value();
    }

    template <class TParam1>
    value_type& construct(TParam1 p1)
    {
        new (&_value) value_type(p1);
        return value();
    }

    template <class TParam1, class TParam2>
    value_type& construct(TParam1 p1, TParam2 p2)
    {
        new (&_value) value_type(p1, p2);
        return value();
    }
#endif

    void destroy()
    {
        value().~value_type();
    }

    operator value_type& () { return value(); }

    operator const value_type& () const { return value(); }

    value_type& operator =(const value_type& copy_from)
    {
        value() = copy_from;
        return value();
    }
};
#endif

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
// Adapted from
// https://stackoverflow.com/questions/9065081/how-do-i-get-the-argument-types-of-a-function-pointer-in-a-variadic-template-cla
// Additional guidance from
// https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda/7943765#7943765
template<typename T>
struct function_traits;

template <typename F, F f>
struct function_ptr_traits;

template<typename R, typename ...Args>
struct function_traits<R(Args...)>
{
    static const size_t nargs = sizeof...(Args);

    typedef R result_type;

    template <size_t i>
    struct arg
    {
        typedef typename estd::tuple_element<i, estd::tuple<Args...>>::type type;
    };

    template <size_t i>
    using arg_t = typename arg<i>::type;

    using tuple = estd::tuple<Args...>;
};

template<typename R, typename ...Args>
struct function_traits<estd::detail::function<R(Args...)> > :
    function_traits<R(Args...)>
{

};

// This one works well for functors from regular functions
template<typename R, typename ...Args>
struct function_traits<R (&)(Args...)> :
    function_traits<R(Args...)>
{

};

// Guidance from
// https://stackoverflow.com/questions/39131137/function-pointer-as-template-argument-and-signature
template<class T, class R, typename ...Args, R (T::*f)(Args...)>
struct function_ptr_traits<R (T::*)(Args...), f> :
    function_traits<R(Args...)>
{
    typedef T this_type;
};

#endif

}

#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)

namespace detail { inline namespace v1 {

/// Lower-level version of function which is hands-off for memory management
template <typename TResult, typename... TArgs, class TImpl>
class function<TResult(TArgs...), TImpl> : public internal::function_base_tag
{
protected:
    typedef TImpl impl_type;

    // DEBT: Don't really like model_base and model as public, but may be necessary
public:
    typedef typename impl_type::model_base model_base;

    // NOTE: c++17 CTAD may be required for this to really be useful
    // https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
    template <class F>
    using model = typename impl_type::template model<F>;

protected:
    // DEBT: child class constructors need this to not be const, for now.  Also, nullability demands it
    //concept* const m;
    model_base* m;

public:
    // DEBT: We need operator = for these two as well, so that we can
    // make them explicit - unless we can prove having just these constructors
    // is a good practice (and document here)
    function(nullptr_t = nullptr_t{}) : m(NULLPTR) {}
    function(model_base* m) : m(m) {}

    function(const function& copy_from) = default;

    // NOTE: If we decide to add nullability to function_base, then a move constructor
    // makes sense
    function(function&& move_from) = delete;

    function& operator =(const function&) = default;

    // NOTE: Removed separate && version since it was an ambiguous overload, and
    // std::function only has this kind of signature anyway
    // https://en.cppreference.com/w/cpp/utility/functional/function/operator()
    inline TResult operator()(TArgs... args)
    {
        // a little complicated.  Some guidance from:
        // https://stackoverflow.com/questions/2402579/function-pointer-to-member-function
        // the first portion m->* indicates that a method function pointer call is happening
        // and to load in 'm' to the 'this' pointer.  The (m->f) portion actually retrieves
        // the function pointer itself
        //return (m->*(m->f))(std::forward<TArgs>(args)...);

        // DEBT: Prefer lower overhead of above mess, but while we diagnose ESP32 failures
        // let's make our lives easier
        return (*m)(std::forward<TArgs>(args)...);
    }

    explicit operator bool() const NOEXCEPT { return m != NULLPTR; }

    // See above 'model' CTAD comments
    template <typename F>
    inline static model<F> make_model(F&& f)
    {
        return model<F>(std::forward<F>(f));
    }

    // EXPERIMENTAL
    const model_base* getm() const { return m; }

    // EXPERIMENTAL
    template <template <template <class> class, typename F, class ...TArgs2> class TProvided, class ...TArgs2>
    using imbue = TProvided<detail::impl::function_fnptr1, TResult(TArgs...), TArgs2...>;
};

}}

namespace internal {

// DEBT: Might be better named as 'method', except that could be somewhat ambiguous
template <typename Result, typename... Args, template <class> class Impl>
class thisify_function<Result(Args...), Impl> :
    public detail::v2::function<Result(Args...), Impl>
{
    using base_type = detail::v2::function<Result(Args...), Impl>;
    using provider_type =  internal::impl::function_context_provider<Impl, Result(Args...)>;

public:
    template <class T>
    using function_type = typename provider_type::template function_type<T>;

    template <class T, function_type<T> f>
    using model = typename provider_type::template model<T, f>;

private:
    // This model exists specifically to accomodate overlay/union of specific model
    // onto placeholder
    template <class T, function_type<T> f>
    struct overlay_model : model<T, f>
    {
        typedef model<T, f> base_type;

        // NOTE: This is a bizarre thing we do here.  We take advantage of the fact that pointer
        // sizes don't change and accept a foreign-typed model.  We do this to simulate a runtime
        // templated union initialization.  base type gets initialized with a constant pointer to
        // foreign model's exec helper, and naturally we copy over the 'this'.  I would not be
        // surprised if this falls into "undefined" behavior at some point, but in the end we are
        // only relying on 2 runtime pointers to not change size and 2 compile time pointers
        template <class T2, function_type<T2> f2>
        constexpr overlay_model(const model<T2, f2>& copy_from) :
            base_type(
                (T*)copy_from.foreign_this,
                static_cast<typename base_type::function_type>(&model<T2, f2>::exec))
        {
        }
    };

    /* Nifty but not useful here
    template <class T, function_type<T> f>
    union holder
    {
        model<T, f> m;
    };*/

    struct placeholder_result
    {
        Result noop(Args...args) { return Result{}; }
    };

    struct placeholder_void
    {
        void noop(Args...args) {}
    };

    typedef estd::conditional_t<estd::is_void<Result>::value,
            placeholder_void,
            placeholder_result> placeholder;

    overlay_model<placeholder, &placeholder::noop> m_;

public:
    /*
     * NOTE: Not usable because 'f' can never be deduced
    template <class T, function_type<T> f>
    context_function(T* foreign_this)
    {
    } */

    template <class T, function_type<T> f>
    constexpr explicit thisify_function(const model<T, f>& m) :
        base_type(&m_),
        m_(m)
    {
    }

    // UNUSED, and you're probably better off using inline_function
    template <class T, function_type<T> f>
    static thisify_function create(T* foreign_this)
    {
        return thisify_function(model<T, f>(foreign_this));
    }
};

// Like a dumbed-down bind
template <typename TResult, typename... TArgs, typename... TContexts>
class contextify_function<TResult(TArgs...), TContexts...> :
    public detail::function<TResult(TArgs...), detail::impl::function_fnptr1<TResult(TArgs...)>>
{
    typedef detail::function<TResult(TArgs...), detail::impl::function_fnptr1<TResult(TArgs...)>> base_type;

public:
    template <class T>
    using function_type = TResult (*)(TArgs..., T, TContexts...);

    template <class T, function_type<T> f>
    class model : public base_type::model_base
    {
        estd::tuple<T, TContexts...> contexts;

    public:
        model(T&& t, TContexts&&... contexts) :
            base_type::model_base(static_cast<typename base_type::model_base::function_type>(&model::exec)),
            contexts(std::move(t), std::forward<TContexts>(contexts)...)
        {

        }

        // DEBT: Doesn't yet handle void return
        TResult exec(TArgs...args)
        {
            return estd::apply([&](T t, TContexts...c)
            {
                return f(args..., t, c...);
            },
            contexts);
            //return f(std::forward<TArgs>(args)..., get<0>(contexts));
        }
    };
};

// DEBT: Move this to 'impl' namespace
template <class TImpl, class F>
inline typename TImpl::template model<F> make_model_impl(F&& f)
{
    return typename TImpl::template model<F>(std::move(f));
}

// DEBT: Hard wired to detail::impl::function_default.  Probably OK as long as we
// maintain a separate explicit impl flavor
template <class TSignature, class F>
inline typename detail::impl::function_default<TSignature>::template model<F> make_model(F&& f)
{
    return make_model_impl<detail::impl::function_default<TSignature> >(std::move(f));
}

}

#endif

}
