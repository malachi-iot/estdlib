#pragma once

#include "platform.h"
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


template <class T, class E>
class expected
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

    ESTD_CPP_DEFAULT_CTOR(expected)

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