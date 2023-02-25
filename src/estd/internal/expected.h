#pragma once

#include "platform.h"
#include "type_traits.h"
#include "variant.h"

#include "raw/utility.h"
#include "raw/expected.h"

namespace estd { namespace internal {

template <class E>
class unexpected
{
private:
    E error_;

protected:
    typedef E error_type;

    ESTD_CPP_DEFAULT_CTOR(unexpected)

#if __cpp_variadic_templates
    template <class Err = E>
    constexpr explicit unexpected(Err&& e) : error_(std::move(e)) {}

    template <class ...TArgs>
    constexpr explicit unexpected(in_place_t, TArgs&&...args) :
        error_(std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET unexpected(const TE1& e) : error_(e) {}
#endif

public:
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }
};

template <class T, class E, class enabled = void>
class expected;

template <class T, class E, class enabled = void>
union expected_storage;

template <class T, class E>
union expected_storage<T, E,
#if FEATURE_ESTD_IS_TRIVIAL
    typename enable_if<estd::is_trivial<T>::value>::type >
#elif FEATURE_STD_TYPE_TRAITS
typename enable_if<std::is_trivial<T>::value>::type >
#else
    int>
#endif
{

};


template <class T, class E>
union expected_storage<T, E,
#if FEATURE_ESTD_IS_TRIVIAL
    typename enable_if<!estd::is_trivial<T>::value>::type >
#elif FEATURE_STD_TYPE_TRAITS
    typename enable_if<!std::is_trivial<T>::value>::type >
#else
    void>
#endif
{
    typedef estd::byte value_placeholder_type[sizeof(T)];
    typedef estd::byte error_placeholder_type[sizeof(E)];

    value_placeholder_type value_;
    error_placeholder_type error_;

    T* value() { return (T*)&value_; }
    E* error() { return (E*)&error_; }
};


template <class T, class E>
class expected<T, E,
#if FEATURE_ESTD_IS_TRIVIAL
    typename enable_if<estd::is_trivial<T>::value>::type >
#elif FEATURE_STD_TYPE_TRAITS
    typename enable_if<std::is_trivial<T>::value>::type >
#else
    // When not able to determine triviality, default to assuming it IS trivial
    void>
#endif
{
public:
    typedef T value_type;
    typedef E error_type;

protected:
    typedef T nonvoid_value_type;

    union
    {
        value_type value_;
        error_type error_;
    };

    ESTD_CPP_CONSTEXPR_RET expected() : value_{} {}

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(in_place_t, TArgs&&...args) :
        value_(std::forward<TArgs>(args)...)
    {}

    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        error_(std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) : error_(e) {}
#endif

    ESTD_CPP_CONSTEXPR_RET expected(const value_type& v) : value_(v) {}

public:
    T& value() { return value_; }
    ESTD_CPP_CONSTEXPR_RET const T& value() const { return value_; }

    E& error() { return error_; }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }

    const T& operator*() const { return value_; }

    expected& operator=(value_type&& v)
    {
        value_ = v;
    }
};


template <class T, class E>
class expected<T, E,
#if FEATURE_ESTD_IS_TRIVIAL
    typename enable_if<!estd::is_trivial<T>::value>::type >
#elif FEATURE_STD_TYPE_TRAITS
    typename enable_if<!std::is_trivial<T>::value>::type >
#else
    int>
#endif
{
    expected_storage<T, E> storage;

protected:
    typedef T nonvoid_value_type;

#if __cpp_variadic_templates
    template <class... TArgs>
    explicit expected(in_place_t, TArgs&&...args)
    {
        new (storage.value()) T(std::forward<TArgs>(args)...);
    }

    template <class... TArgs>
    explicit expected(unexpect_t, TArgs&&...args)
    {
        new (storage.error()) E(std::forward<TArgs>(args)...);
    }

    explicit expected()
    {
        new (storage.value()) T();
    }
#endif

public:
    typedef T value_type;
    typedef E error_type;

    T& value() { return *storage.value(); }
    E& error() { return *storage.error(); }
};

// DEBT: We'd like to do a const E here, but that demands initializing error()
template <class E>
class expected<void, E> : public unexpected<E>
{
    typedef unexpected<E> base_type;

public:
    typedef void value_type;
    typedef estd::monostate nonvoid_value_type;
    typedef E error_type;

protected:
    ESTD_CPP_DEFAULT_CTOR(expected)

#if __cpp_variadic_templates
    template <class... TArgs>
    constexpr explicit expected(unexpect_t, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...)
    {}
#else
    template <class TE1>
    ESTD_CPP_CONSTEXPR_RET expected(unexpect_t, const TE1& e) : base_type(e) {}
#endif

public:
    static void value() { }
};

}}