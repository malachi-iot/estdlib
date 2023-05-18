#pragma once

#include "platform.h"
#include "type_traits.h"
#include "utility.h"
#include "variant.h"

#include "raw/expected.h"

namespace estd { namespace internal {

// Doesn't need to play with uninitialized storage
// since it's always required that E is initialized somehow
// when using this directly as 'unexpected'
// However, when using it indirectly from the expected void specialization,
// it DOES want to keep this uninitialized sometimes.  So, what we really
// need is that flavor of expected to have a specialized expected_storage
// to match (to handle trivial and non trivial)
template <class E>
class unexpected
{
private:
    E error_;

protected:
    typedef E error_type;

    ESTD_CPP_DEFAULT_CTOR(unexpected)

    // If we were to  do const E, this is needed
    //ESTD_CPP_CONSTEXPR_RET unexpected() : error_() {}

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

template <class T, class E, bool trivial>
union expected_storage;

template <class T, class E>
struct expected_is_trivial;


template <class T, class E, bool trivial =
          expected_is_trivial<T, E>::is_trivial>
class expected;

template <class T, class E>
struct expected_is_trivial
{
    // DEBT: Do this all with an integral constant, just having a brain
    // fart how to do an or operation in that context
    static CONSTEXPR bool is_trivial =
#if FEATURE_ESTD_IS_TRIVIAL
        estd::is_trivial<T>::value & estd::is_trivial<E>::value;
#elif FEATURE_STD_TYPE_TRAITS
        std::is_trivial<T>::value & std::is_trivial<E>::value;
#else
        false;
#endif
};

template <class E>
struct expected_is_trivial<void, E>
{
    static CONSTEXPR bool is_trivial =
#if FEATURE_ESTD_IS_TRIVIAL
        estd::is_trivial<E>::value;
#elif FEATURE_STD_TYPE_TRAITS
        std::is_trivial<E>::value;
#else
        false;
#endif
};

// DEBT: We can't really use estd::layer1::optional here as E and T
// overlap, but perhaps we can use a high-bit filter in controlled
// circumstances

template <class E>
union expected_storage<void, E, true>
{
    E error_;

    E* error() { return &error_; }
    void value() {}
};

template <class T, class E>
union expected_storage<T, E, true>
{
    T value_;
    E error_;

    T* value() { return &value_; }
    E* error() { return &error_; }
};


// DEBT: Not yet used
template <class E>
union expected_storage<void, E, false>
{
    typedef estd::byte error_placeholder_type[sizeof(E)];

    error_placeholder_type error_;

    E* error() { return (E*)&error_; }
    void value() {}
};



template <class T, class E>
union expected_storage<T, E, false>
{
    typedef estd::byte value_placeholder_type[sizeof(T)];
    typedef estd::byte error_placeholder_type[sizeof(E)];

    value_placeholder_type value_;
    error_placeholder_type error_;

    const T* value() const { return (const T*)&value_; }
    T* value() { return (T*)&value_; }
    E* error() { return (E*)&error_; }

    expected_storage() = default;
    /*
    expected_storage(const T& copy_t) :
        (*((T*)&value_)){copy_t}
    {} */
};


// Trivial flavor
template <class T, class E>
class expected<T, E, true>
{
public:
    typedef T value_type;
    typedef E error_type;

protected:
    typedef T nonvoid_value_type;

    //variant_storage2<T, E> storage;

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


// Non trivial flavor
template <class T, class E>
class expected<T, E, false>
{
    expected_storage<T, E, false> storage;

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
#endif

    // DEBT: Figure out how to make this and others work on initializing list line
    // so that we can constexpr it
    explicit expected(const nonvoid_value_type& v)
    {
        *storage.value() = v;
    }

    explicit expected()
    {
        new (storage.value()) T();
    }

public:
    typedef T value_type;
    typedef E error_type;

    T& value() { return *storage.value(); }
    E& error() { return *storage.error(); }

    const T& operator*() const { return *storage.value(); }
};

// DEBT: We'd like to do a const E here, but that demands initializing error()
// DEBT: trivial not truly handled here, assumes always trivial really.  Use
// expected_storage instead of deriving from 'unexpected'
template <class E>
class expected<void, E, true> : public unexpected<E>
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
