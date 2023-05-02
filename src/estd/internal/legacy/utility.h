#pragma once

#include "../platform.h"

// DEBT: The following ESTD_FN are phased out/obsolete

// semi-formalization of https://stackoverflow.com/questions/7834226/detecting-typedef-at-compile-time-template-metaprogramming
#define ESTD_FN_HAS_TYPEDEF_EXP(typedef_name) \
template<typename T, typename = void>   \
struct has_##typedef_name##_typedef : estd::false_type {}; \
    \
template<typename T> \
struct has_##typedef_name##_typedef<T, typename estd::internal::has_typedef<typename T::typedef_name>::type> : estd::true_type {};

#define ESTD_FN_HAS_TAG_EXP(tag_name_minus_suffix) \
template<typename T, typename = void>   \
struct has_##tag_name_minus_suffix##_tag : estd::false_type {}; \
    \
template<typename T> \
struct has_##tag_name_minus_suffix##_tag<T, typename estd::internal::has_typedef<typename T::tag_name_minus_suffix##_tag>::type> : estd::true_type {};

// MethodInfo usage is to resolve base classes
// don't know at this time how to do this without using decltype
#ifdef FEATURE_CPP_DECLTYPE
#define ESTD_FN_HAS_METHOD_EXP(ret_type, method_name, ...) \
    template <typename C> static CONSTEXPR estd::internal::has_member_base::yes& test_has_##method_name##_ \
        (estd::internal::has_member_base::reallyHas<ret_type ( \
        estd::internal::MethodInfo<decltype(&C::method_name)>::ClassType::*)(__VA_ARGS__), &C::method_name>* /*unused*/) \
    { return estd::internal::has_member_base::yes_value; }  \
\
    template <typename C> static CONSTEXPR estd::internal::has_member_base::yes& test_has_##method_name##_ \
        (estd::internal::has_member_base::reallyHas<ret_type ( \
        estd::internal::MethodInfo<decltype(&C::method_name)>::ClassType::*)(__VA_ARGS__) const, &C::method_name>* /*unused*/) \
    { return estd::internal::has_member_base::yes_value; }  \
\
    template <typename> static CONSTEXPR estd::internal::has_member_base::no& test_has_##method_name##_(...) \
        { return estd::internal::has_member_base::no_value; }

#define ESTD_FN_HAS_PROTECTED_METHOD_EXP(ret_type, method_name, ...) \
ESTD_FN_HAS_METHOD_EXP(ret_type, method_name, __VA_ARGS__) \
\
template <class T> \
struct has_##method_name##_method \
{ \
    static CONSTEXPR bool value = sizeof(test_has_##method_name##_<T>(nullptr)) == sizeof(estd::internal::has_member_base::yes); \
};

#define ESTD_FN_HAS_METHOD(ret_type, method_name, ...) \
template <class T> struct has_##method_name##_method : estd::internal::has_member_base \
{ \
    ESTD_FN_HAS_METHOD_EXP(ret_type, method_name, __VA_ARGS__) \
\
    static CONSTEXPR bool value = sizeof(test_has_##method_name##_<T>(nullptr)) == sizeof(yes); \
};
#else
#define ESTD_FN_HAS_METHOD(ret_type, method_name, ...) \
template <class T> struct has_##method_name##_method : estd::internal::has_member_base \
{ \
    template <typename C> static CONSTEXPR yes& test(reallyHas<ret_type ( \
        C::*)(__VA_ARGS__), &C::method_name>* /*unused*/) \
    { return yes_value; }  \
\
    template <typename C> static CONSTEXPR yes& test(reallyHas<ret_type ( \
        C::*)(__VA_ARGS__) const, &C::method_name>* /*unused*/) \
    { return yes_value; }  \
\
    template <typename> static CONSTEXPR no& test(...) { return no_value; } \
\
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes); \
};
#endif
