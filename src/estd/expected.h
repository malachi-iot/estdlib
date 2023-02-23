#pragma once

#include "internal/platform.h"

namespace estd {

template <class E>
class unexpected
{
private:
    const E error_;

protected:
    ESTD_CPP_DEFAULT_CTOR(unexpected)

public:
    unexpected(E e) : error_(e) {}

    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }
};

template <class T, class E>
class expected
{
public:
    typedef T value_type;
    typedef E error_type;
    typedef unexpected<E> unexpected_type;

private:
    union
    {
        const value_type value_;
        const error_type error_;
    };

    const bool has_value_;

public:
    ESTD_CPP_CONSTEXPR_RET expected() :
        value_(),
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR_RET expected(T v) :
        value_(v),
        has_value_(true)
    {}

    ESTD_CPP_CONSTEXPR_RET expected(E e) :
        error_(e),
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR_RET expected(unexpected<E> u) :
        error_(u.error()),
        has_value_(false)
    {}

    T& value() { return value_; }
    E& error() { return error_; }
    ESTD_CPP_CONSTEXPR_RET const T& value() const { return value_; }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }

    bool has_value() const { return has_value_; }
#if __cpp_constexpr
    constexpr explicit
#endif
    operator bool() const { return has_value_; }

    const T& operator*() const { return value_; }
};


template <class E>
class expected<void, E> : public unexpected<E>
{
    typedef unexpected<E> base_type;

public:
    ESTD_CPP_DEFAULT_CTOR(expected)
    ESTD_CPP_CONSTEXPR_RET expected(E e) : unexpected<E>(e) {}

    static void value() {}
};

}