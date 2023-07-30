// See README.md in this folder

#pragma once

// Remember, initializer_lists are not forwarded!
// https://stackoverflow.com/questions/28370970/forwarding-initializer-list-expressions
#if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)
#define ESTD_CPP_FORWARDING_CTOR_MEMBER(class_name, member)    \
    template <class ...FwdArgs>                   \
    explicit constexpr class_name(FwdArgs&&...args) :      \
        member(std::forward<FwdArgs>(args)...) \
    {}
#else
#define ESTD_CPP_FORWARDING_CTOR_MEMBER(class_name, member)    \
    template <class TParam1>                    \
    class_name(const TParam1& p1) :             \
        member(p1) {}
#endif

#define ESTD_CPP_FORWARDING_CTOR(class_name)    ESTD_CPP_FORWARDING_CTOR_MEMBER(class_name, base_type)

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

#if __cpp_ref_qualifiers
#define ESTD_CPP_REFQ &
#else
#define ESTD_CPP_REFQ
#endif

// Assistance to define typical "typedef T value_type" and friends
#define ESTD_CPP_STD_VALUE_TYPE(T)  \
    typedef T value_type;           \
    typedef value_type& reference;  \
    typedef const value_type& const_reference;  \
    typedef value_type* pointer;                \
    typedef const value_type* const_pointer;

/// Wrapper for c++20 concepts for use in template lists
#if __cpp_concepts
#define ESTD_CPP_CONCEPT(T)     T
#else
#define ESTD_CPP_CONCEPT(T)     class
#endif
