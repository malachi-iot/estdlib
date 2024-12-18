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

// Take pages out of GCC playbook
// DEBT: These belong elsewhere, not in c++_emul per se
#if __cplusplus >= 202002L
#define ESTD_CPP20_CONSTEXPR constexpr
#else
#define ESTD_CPP20_CONSTEXPR
#endif

#if __cplusplus >= 201703L
#define ESTD_CPP17_CONSTEXPR constexpr
#else
#define ESTD_CPP17_CONSTEXPR
#endif

#if __cplusplus >= 201402L
#define ESTD_CPP14_CONSTEXPR constexpr
#else
#define ESTD_CPP14_CONSTEXPR
#endif


#define ESTD_CPP_CONSTEXPR(v)    ESTD_CPP ## v ## _CONSTEXPR

#if __cpp_ref_qualifiers
#define ESTD_CPP_REFQ &
#else
#define ESTD_CPP_REFQ
#endif

/// Wrapper for c++20 concepts for use in template lists
#if __cpp_concepts
#define ESTD_CPP_CONCEPT(T)     T
#else
#define ESTD_CPP_CONCEPT(T)     class
#endif
