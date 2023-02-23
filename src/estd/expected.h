#pragma once

#include "internal/platform.h"

namespace estd {

namespace internal {

template <class E>
class unexpected
{
private:
    E error_;

protected:
    typedef E error_type;

    ESTD_CPP_DEFAULT_CTOR(unexpected)
    ESTD_CPP_CONSTEXPR_RET unexpected(error_type e) : error_(e) {}

public:
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }
};


template <class T, class E>
class expected
{
public:
    typedef T value_type;
    typedef unexpected<E> error_type;

protected:
    union
    {
        const value_type value_;
        const error_type error_;
    };

    ESTD_CPP_DEFAULT_CTOR(expected)
    ESTD_CPP_CONSTEXPR_RET expected(error_type e) : error_(e) {}
    ESTD_CPP_CONSTEXPR_RET expected(value_type v) : value_(v) {}

public:
    T& value() { return value_; }
    ESTD_CPP_CONSTEXPR_RET const T& value() const { return value_; }

    E& error() { return error_; }
    ESTD_CPP_CONSTEXPR_RET const E& error() const { return error_; }

    const T& operator*() const { return value_; }
};

// DEBT: We'd like to do a const E here, but that demands initializing error()
template <class E>
class expected<void, E> : public unexpected<E>
{
    typedef unexpected<E> base_type;

public:
    typedef void value_type;
    typedef E error_type;

protected:
    ESTD_CPP_DEFAULT_CTOR(expected)
    ESTD_CPP_CONSTEXPR_RET expected(error_type e) : base_type(e) {}

public:
    static void value() { }
};

}

template <class E>
class unexpected : public internal::unexpected<const E>
{
    typedef internal::unexpected<const E> base_type;

public:
    ESTD_CPP_CONSTEXPR_RET unexpected(E e) : base_type(e) {}
};


template <class T, class E>
class expected : public internal::expected<T, E>
{
    typedef internal::expected<T, E> base_type;

    const bool has_value_;

public:
    ESTD_CPP_CONSTEXPR_RET expected() :
        has_value_(true)
    {}

    template <class T2>
    ESTD_CPP_CONSTEXPR_RET expected(T2 v) :
        base_type(v),
        has_value_(true)
    {}

    ESTD_CPP_CONSTEXPR_RET expected(E e) :
        base_type(e),
        has_value_(false)
    {}

    ESTD_CPP_CONSTEXPR_RET expected(unexpected<E> u) :
        base_type(u.error()),
        has_value_(false)
    {}

    bool has_value() const { return has_value_; }
#if __cpp_constexpr
    constexpr explicit
#endif
    operator bool() const { return has_value_; }
};


}