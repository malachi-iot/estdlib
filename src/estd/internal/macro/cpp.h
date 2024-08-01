#pragma once

// NOTE: Only applies to empty constructor.  You're on your own for the "more complex" varieties
#ifdef FEATURE_CPP_DEFAULT_CTOR
#define ESTD_CPP_DEFAULT_CTOR(class_name)   constexpr class_name() = default;
#else
#define ESTD_CPP_DEFAULT_CTOR(class_name)   class_name() {}
#endif

// Assistance to define typical "typedef T value_type" and friends
#define ESTD_CPP_STD_VALUE_TYPE(T)  \
typedef T value_type;           \
    typedef value_type& reference;  \
    typedef const value_type& const_reference;  \
    typedef value_type* pointer;                \
    typedef const value_type* const_pointer;

// NOTE: Not using feature test has_macro since I've seen that glitch
// out in llvm IIRC
#if __cplusplus >= 201703L
#define ESTD_CPP_ATTR_FALLTHROUGH   [[fallthrough]]
#define ESTD_CPP_ATTR_NODISCARD     [[nodiscard]]
#else
#define ESTD_CPP_ATTR_FALLTHROUGH
#define ESTD_CPP_ATTR_NODISCARD
#endif
