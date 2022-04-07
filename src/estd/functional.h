#pragma once

#include "internal/platform.h"
#include "internal/functional.h"

#include "tuple.h"
#include "type_traits.h"

#include "internal/invoke.h"

// TODO: Utilize std version of this, if available

namespace estd {

template <class T>
class reference_wrapper {
public:
    // types
    typedef T type;

    // construct/copy/destroy
    reference_wrapper(T& ref) NOEXCEPT : _ptr(estd::addressof(ref)) {}
#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_wrapper(T&&) = delete;
#endif
#ifdef FEATURE_CPP_DEFAULT_CTOR
    reference_wrapper(const reference_wrapper&) NOEXCEPT = default;

    // assignment
    reference_wrapper& operator=(const reference_wrapper& x) NOEXCEPT = default;
#else
    reference_wrapper(const reference_wrapper& copy_from) NOEXCEPT :
    _ptr(copy_from._ptr)
  {};

  // assignment
  reference_wrapper& operator=(const reference_wrapper& copy_from) NOEXCEPT
  {
    _ptr = copy_from._ptr;
  }
#endif

    // access
    operator T& () const NOEXCEPT { return *_ptr; }
    T& get() const NOEXCEPT { return *_ptr; }

    /*
    template< class... ArgTypes >
    estd::invoke_result_t<T&, ArgTypes...>
      operator() ( ArgTypes&&... args ) const {
      return estd::invoke(get(), std::forward<ArgTypes>(args)...);
    } */

private:
    T* _ptr;
};


#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)

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
                std::forward<TArgs...>(args...)
                );

    return b;
}

#endif




#ifdef FEATURE_CPP_DEDUCTION_GUIDES
// deduction guides
template<class T>
reference_wrapper(reference_wrapper<T>) -> reference_wrapper<T>;
#endif

template <class T>
reference_wrapper<T> ref(T& t) NOEXCEPT
{
    return reference_wrapper<T>(t);
}

template <class T>
reference_wrapper<T> cref(const T& t) NOEXCEPT
{
    return reference_wrapper<T>(t);
}


namespace experimental {

// Guidance from
// https://stackoverflow.com/questions/14936539/how-stdfunction-works

template <typename T>
class function;

template <typename TResult, typename... TArgs>
class function_base
{
    typedef TResult (*function_type)(void*, TArgs...);

protected:
    function_type f;
    void* m;

protected:
    function_base() : f(NULLPTR), m(NULLPTR) {}
    function_base(function_type f) : f(f) {}

public:
    TResult operator()(TArgs... args)
    {
        return (*f)(this, std::forward<TArgs>(args)...);
    }
};

template <typename TResult, typename... TArgs>
class function<TResult(TArgs...)> : public function_base<TResult, TArgs...>
{
    typedef function_base<TResult, TArgs...> base_type;

    template <typename T>
    struct model
    {
        template <typename U>
        model(U&& u) : t(std::forward<U>(u)) {}

        T t;

        TResult exec(TArgs...args) const
        {
            return t(std::forward<TArgs>(args)...);
        }

        static TResult exec(void* _this, TArgs...args)
        {
            return ((model*)_this)->exec(std::forward<TArgs>(args)...);
        }
    };

public:
    template <typename T>
    function(T&& t)
    {
        //m = allocator_traits<model<T>>::
        typedef typename std::decay<T>::type incoming_function_type;
        typedef model<incoming_function_type> model_type;
        // FIX: Don't want to dynamically allocate memory quite this way
        auto m = new model_type(std::forward<T>(t));
        base_type::m = m;
        base_type::f = &model_type::exec;
    }

    function(const function& other) = default;

#ifdef FEATURE_CPP_MOVESEMANTIC
    function(function&& other) NOEXCEPT
    {
        *this = std::move(other);
    }
#endif

    ~function()
    {
        // FIX: Don't want to dynamically allocate memory quite this way, and delete void*
        // rightly gives us complaints and warnings
        delete base_type::m;
    }

    template <typename T>
    function& operator=(T&& t)
    {
        //m = new model<typename std::decay<T>::type >(std::forward<T>(t));
        new (this) function(std::move(t));
        return *this;
    }
};

}

}
