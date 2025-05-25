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
    constexpr class_name(std::initializer_list<T> list) : base_type(list) {}
#else
#define ESTD_CPP_FORWARDING_CTOR_LIST(T, class_name)
#endif
