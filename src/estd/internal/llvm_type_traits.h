#pragma once

// just for tooltips/really type_traits in fact includes llvm_type_traits
#include "../type_traits.h"

// code chunks yanked and adapted straight from LLVM

namespace estd {

#if !defined(_LIBCPP_CXX03_LANG)
template <class _Tp> _Tp* addressof(const _Tp&&) noexcept = delete;
#endif

template <class _Tp> struct alignment_of
    : public integral_constant<size_t, __alignof__(_Tp)> {};

namespace internal {


template <class _Tp, class _Up, bool = is_const<typename remove_reference<_Tp>::type>::value,
                             bool = is_volatile<typename remove_reference<_Tp>::type>::value>
struct __apply_cv
{
    typedef _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, true, false>
{
    typedef const _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, false, true>
{
    typedef volatile _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, true, true>
{
    typedef const volatile _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, false, false>
{
    typedef _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, true, false>
{
    typedef const _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, false, true>
{
    typedef volatile _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, true, true>
{
    typedef const volatile _Up& type;
};


template <class _Tp> struct __libcpp_is_void       : public false_type {};
template <>          struct __libcpp_is_void<void> : public true_type {};

template <class _Hp, class _Tp>
struct __type_list
{
    typedef _Hp _Head;
    typedef _Tp _Tail;
};

template <class _Tp>
struct __align_type
{
    static const size_t value = alignment_of<_Tp>::value;
    typedef _Tp type;
};


struct __nat
{
#ifdef FEATURE_CPP_DELETE_CTOR
    __nat() = delete;
    __nat(const __nat&) = delete;
    __nat& operator=(const __nat&) = delete;
    ~__nat() = delete;
#endif
};


struct __struct_double {long double __lx;};
struct __struct_double4 {double __lx[4];};


typedef
    __type_list<__align_type<unsigned char>,
    __type_list<__align_type<unsigned short>,
    __type_list<__align_type<unsigned int>,
    __type_list<__align_type<unsigned long>,
    __type_list<__align_type<unsigned long long>,
    __type_list<__align_type<double>,
    __type_list<__align_type<long double>,
    __type_list<__align_type<__struct_double>,
    __type_list<__align_type<__struct_double4>,
    __type_list<__align_type<int*>,
    __nat
    > > > > > > > > > > __all_types;

typedef
    __type_list<signed char,
    __type_list<signed short,
    __type_list<signed int,
    __type_list<signed long,
    __type_list<signed long long,
#ifdef FEATURE_CPP_INT128
    __type_list<__int128_t,
#endif
    __nat
// old flag was !_LIBCPP_HAS_NO_INT128
#ifdef FEATURE_CPP_INT128
    >
#endif
    > > > > > __signed_types;
}

template <class _Tp> struct is_void
    : public internal::__libcpp_is_void<typename remove_cv<_Tp>::type> {};

// original also had a clang-specific check to use an inbuild __is_enum
// but we don't follow suit, at least not for now
template <class _Tp> struct is_enum
    : public integral_constant<bool, !is_void<_Tp>::value             &&
                                     !is_integral<_Tp>::value         &&
                                     !is_floating_point<_Tp>::value   &&
                                     !is_array<_Tp>::value            &&
                                     !is_pointer<_Tp>::value          &&
                                     !is_reference<_Tp>::value        &&
                                     //!is_member_pointer<_Tp>::value   &&
                                     //!is_union<_Tp>::value            &&
#ifdef FEATURE_CPP_VARIADIC
                                    !is_function<_Tp>::value         &&
#endif
                                     !is_class<_Tp>::value
        > {};

namespace internal {

template <class _Tp> struct __libcpp_is_integral                     : public false_type {};
template <>          struct __libcpp_is_integral<bool>               : public true_type {};
template <>          struct __libcpp_is_integral<char>               : public true_type {};
template <>          struct __libcpp_is_integral<signed char>        : public true_type {};
template <>          struct __libcpp_is_integral<unsigned char>      : public true_type {};
template <>          struct __libcpp_is_integral<wchar_t>            : public true_type {};
#ifdef FEATURE_CPP_CHAR16_T
template <>          struct __libcpp_is_integral<char16_t>           : public true_type {};
template <>          struct __libcpp_is_integral<char32_t>           : public true_type {};
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
template <>          struct __libcpp_is_integral<short>              : public true_type {};
template <>          struct __libcpp_is_integral<unsigned short>     : public true_type {};
template <>          struct __libcpp_is_integral<int>                : public true_type {};
template <>          struct __libcpp_is_integral<unsigned int>       : public true_type {};
template <>          struct __libcpp_is_integral<long>               : public true_type {};
template <>          struct __libcpp_is_integral<unsigned long>      : public true_type {};
template <>          struct __libcpp_is_integral<long long>          : public true_type {};
template <>          struct __libcpp_is_integral<unsigned long long> : public true_type {};
#ifdef FEATURE_CPP_INT128
template <>          struct __libcpp_is_integral<__int128_t>         : public true_type {};
template <>          struct __libcpp_is_integral<__uint128_t>        : public true_type {};
#endif

}

template <class _Tp> struct is_integral
    : public internal::__libcpp_is_integral<typename remove_cv<_Tp>::type> {};

namespace internal {

template <class _TypeList, size_t _Size, bool = _Size <= sizeof(typename _TypeList::_Head)> struct __find_first;

template <class _Hp, class _Tp, size_t _Size>
struct __find_first<__type_list<_Hp, _Tp>, _Size, true>
{
    typedef _Hp type;
};

template <class _Hp, class _Tp, size_t _Size>
struct __find_first<__type_list<_Hp, _Tp>, _Size, false>
{
    typedef typename __find_first<_Tp, _Size>::type type;
};


template <class _Tp, bool = is_integral<_Tp>::value || is_enum<_Tp>::value>
struct __make_signed {};

template <class _Tp>
struct __make_signed<_Tp, true>
{
    typedef typename __find_first<__signed_types, sizeof(_Tp)>::type type;
};


template <> struct __make_signed<  signed short,     true> {typedef short     type;};
template <> struct __make_signed<unsigned short,     true> {typedef short     type;};
template <> struct __make_signed<  signed int,       true> {typedef int       type;};
template <> struct __make_signed<unsigned int,       true> {typedef int       type;};
template <> struct __make_signed<  signed long,      true> {typedef long      type;};
template <> struct __make_signed<unsigned long,      true> {typedef long      type;};
template <> struct __make_signed<  signed long long, true> {typedef long long type;};
template <> struct __make_signed<unsigned long long, true> {typedef long long type;};

}

template <class _Tp>
struct make_signed
{
    typedef typename internal::__apply_cv<_Tp, typename internal::__make_signed<typename remove_cv<_Tp>::type>::type>::type type;
};

}
