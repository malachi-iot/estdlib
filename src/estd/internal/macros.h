// These are utility wrappers for forward and backward compatibility
#pragma once

#include "feature/std.h"
#include "feature/cpp.h"

// C++11-ish wrapper section

// NOTE: Only applies to empty constructor.  You're on your own for the "more complex" varieties
#ifdef FEATURE_CPP_DEFAULT_CTOR
#define ESTD_CPP_DEFAULT_CTOR(class_name)   constexpr class_name() = default;
#else
#define ESTD_CPP_DEFAULT_CTOR(class_name)   class_name() {}
#endif

// Remember, initializer_lists are not forwarded!
// https://stackoverflow.com/questions/28370970/forwarding-initializer-list-expressions
#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
#define ESTD_CPP_FORWARDING_CTOR(class_name)    \
    template <class ...TArgs>                   \
    constexpr class_name(TArgs&&...args) :      \
        base_type(std::forward<TArgs>(args)...) \
    {}
#else
#define ESTD_CPP_FORWARDING_CTOR(class_name)    \
    class_name() {}                             \
    template <class TParam1>                    \
    class_name(const TParam1& p1) :             \
        base_type(p1) {}
#endif

#if __cpp_initializer_lists
#define ESTD_CPP_FORWARDING_CTOR_LIST(T, class_name)    \
    class_name(std::initializer_list<T> list) : base_type(list) {}
#else
#define ESTD_CPP_FORWARDING_CTOR_LIST(T, class_name)
#endif

#if __cpp_constexpr
#define ESTD_CPP_CONSTEXPR_RET constexpr
#else
#define ESTD_CPP_CONSTEXPR_RET inline
#endif

// Assistance to define typical "typedef T value_type" and friends
#define ESTD_CPP_STD_VALUE_TYPE(T)  \
    typedef T value_type;           \
    typedef value_type& reference;  \
    typedef const value_type& const_reference;  \
    typedef value_type* pointer;                \
    typedef const value_type* const_pointer;
