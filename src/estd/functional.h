#pragma once

#include "internal/platform.h"
#include "internal/functional.h"
#include "internal/value_evaporator.h"

#include "tuple.h"
#include "type_traits.h"
#include "traits/allocator_traits.h"

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

#if defined (FEATURE_CPP_VARIADIC) && defined (FEATURE_CPP_MOVESEMANTIC)

namespace experimental {

// Guidance from
// https://stackoverflow.com/questions/14936539/how-stdfunction-works

// since 'void' doesn't play nice with reference_evaporator
struct empty_type
{

};

template <typename F, typename TResult, typename... TArgs>
struct model_base
{

};

template <typename T, class TAllocator = empty_type>
class function;

template <typename TResult, typename... TArgs>
class function_base;


template <typename TResult, typename... TArgs>
class function_base<TResult(TArgs...)>
{
protected:
    // function pointer approach works, but if we have to add in a virtual destructor
    // (which is likely) then we are faced with incurring that overhead anyway so might
    // switch over to virtual operator() in that case
    struct concept_fnptr1
    {
        typedef TResult (concept_fnptr1::*function_type)(TArgs&&...);

        function_type const f;

        concept_fnptr1(function_type f) : f(f) {}

        inline TResult _exec(TArgs&&...args)
        {
            return (this->*f)(std::forward<TArgs>(args)...);
        }
    };


    template <typename F>
    struct model_fnptr1 : concept_fnptr1
    {
        typedef concept_fnptr1 base_type;

        //template <typename U>
        model_fnptr1(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model_fnptr1::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        TResult exec(TArgs&&...args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    // this is a slightly less fancy more brute force approach to try to diagose esp32
    // woes
    struct concept_fnptr2
    {
        typedef TResult (*function_type)(void*, TArgs&&...);

        function_type const f;

        concept_fnptr2(function_type f) : f(f) {}

        inline TResult _exec(TArgs&&...args)
        {
            return f(this, std::forward<TArgs>(args)...);
        }
    };

    template <typename F>
    struct model_fnptr2 : concept_fnptr2
    {
        typedef concept_fnptr2 base_type;

        model_fnptr2(F&& u) :
            base_type(static_cast<typename base_type::function_type>(&model_fnptr2::exec)),
            f(std::forward<F>(u))
        {
        }

        F f;

        static TResult exec(void* _this, TArgs&&...args)
        {
            auto __this = ((model_fnptr2*)_this);

            return __this->f(std::forward<TArgs>(args)...);
        }
    };

    struct concept_virtual
    {
        virtual TResult _exec(TArgs&&...args) = 0;
    };

    template <class F>
    struct model_virtual : concept_virtual
    {
        model_virtual(F&& u) :
            f(std::forward<F>(u))
        {
        }

        F f;

        virtual TResult _exec(TArgs&&...args) override
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    typedef concept_fnptr1 concept;
    template <class F>
    using model = model_fnptr1<F>;

    concept* m;

protected:
    //function_base(function_type f) : f(f) {}

public:
    function_base() : m(NULLPTR) {}

    function_base(concept* m) : m(m) {}

    TResult operator()(TArgs&&... args)
    {
        // a little complicated.  Some guidance from:
        // https://stackoverflow.com/questions/2402579/function-pointer-to-member-function
        // the first portion m->* indicates that a method function pointer call is happening
        // and to load in 'm' to the 'this' pointer.  The (m->f) portion actually retrieves
        // the function pointer itself
        //return (m->*(m->f))(std::forward<TArgs>(args)...);

        // DEBT: Prefer lower overhead of above mess, but while we diagnose ESP32 failures
        // let's make our lives easier
        return m->_exec(std::forward<TArgs>(args)...);
    }

    explicit operator bool() const NOEXCEPT { return m != NULLPTR; }

    template <typename F>
    static model<F> make_inline(F&& f)
    {
        return model<F>(std::move(f));
    }
};

template <class TFunc, typename F>
class inline_function;


template <typename TResult, typename... TArgs, class TAllocator>
class function<TResult(TArgs...), TAllocator> :
    public function_base<TResult(TArgs...)>,
    public estd::internal::reference_evaporator < TAllocator, false> //estd::is_class<TAllocator>::value>
{
    typedef function_base<TResult(TArgs...)> base_type;
    typedef typename base_type::concept concept;

    typedef estd::internal::reference_evaporator <TAllocator, false> allocator_provider_type;
    typedef estd::allocator_traits<TAllocator> allocator_traits;

    /*
    template <typename F, typename TAllocator>
    struct model_with_allocator : model<F>
    {
        TAllocator& alloc;

        model_with_allocator(F&& f, TAllocator& alloc) :
            model<F>(std::forward<F>(f)),
            alloc(alloc)
        {

        }
    }; */

private:
    void reset()
    {
        // FIX: Don't want to dynamically allocate memory quite this way, and delete void*
        // rightly gives us complaints and warnings
        delete base_type::m;
    }

public:
    template <typename T>
    function(T&& t) : allocator_provider_type(TAllocator())
    {
        typedef typename estd::decay<T>::type incoming_function_type;
        typedef typename base_type::template model<incoming_function_type> model_type;
        //allocator_traits::rebind_alloc<model_type>
        // FIX: Don't want to dynamically allocate memory quite this way
        auto m = new model_type(std::forward<T>(t));
        base_type::m = m;
    }

    template <typename T>
    function(T&& t, TAllocator& allocator) : allocator_provider_type(allocator)
    {
        typedef typename estd::decay<T>::type incoming_function_type;
        typedef typename base_type::template model<incoming_function_type> model_type;
        auto m = new model_type(std::forward<T>(t));
        base_type::m = m;
    }

    /*
     * Aborting this, thinking estd::function itself is better off taking TAllocator
     * so that 'delete' free operation takes place at 'function' level as it should
     * rather than model level which could be problematic if for example a subclass
     * needed to do more things on destruction.  It also puts off the need for a
     * virtual destructor
    template <typename F, typename TAllocator>
    function(F&& f, TAllocator& alloc)
    {
        typedef estd::allocator_traits<TAllocator> allocator_traits;
        typedef model_with_allocator<typename std::decay<F>::type, TAllocator> model_type;

        auto h = allocator_traits::allocate(alloc, sizeof(model_type));
    } */

    function(const function& other) = default;

#ifdef FEATURE_CPP_MOVESEMANTIC
    function(function&& other) NOEXCEPT :
        allocator_provider_type(std::move(other))
    {
        // TODO
    }
#endif

    ~function()
    {
        reset();
    }

    template <typename F>
    function& operator=(F&& f)
    {
        reset();
        new (this) function(std::move(f));
        return *this;
    }

    template <typename F>
    static inline_function<F, TResult(TArgs...)> make_inline2(F&& f)
    {
        return inline_function<F, TResult(TArgs...)>(std::move(f));
    }
};


template <class TFunc, typename TResult, typename... TArgs>
class inline_function<TFunc, TResult(TArgs...)> : public function_base<TResult(TArgs...)>
{
    typedef function_base<TResult(TArgs...)> base_type;
    typedef typename base_type::template model<TFunc> model_type;

    model_type m;

public:
    inline_function(TFunc&& f) :
        base_type(&m),
        m(std::move(f))
    {

    }
};

template <class TFunc, typename F>
inline_function<TFunc, F> make_inline_function(F&& f)
{
    return inline_function<TFunc, F>();
}

}

#endif  // VARIADIC + MOVE SEMANTIC

}
