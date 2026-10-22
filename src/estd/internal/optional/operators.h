#include "../fwd/optional.h"
#include "../raw/type_traits.h"

namespace estd {

template <class T, class T2, class Base, class Base2>
constexpr bool operator==(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{
    return lhs.has_value() && rhs.has_value() ?
        *lhs == *rhs :
        !lhs.has_value() && !rhs.has_value();
}

template <class T, class U, class TBase>
constexpr bool operator!=(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? *opt != value : true;
}

namespace internal {

template <class T>
struct is_optional : false_type {};

template <class T, class Base>
struct is_optional<optional<T, Base>> : true_type {};

/// @tparam less_than set to true when operator is <, false otherwise
/// @param lhs
/// @param rhs
/// @param f
/// @return
/// @remarks https://en.cppreference.com/cpp/utility/optional/operator_cmp
template <bool less_than = false, class T, class Base1, class U, class Base2, class F>
ESTD_CPP_CONSTEXPR(14) bool optional_compare(
    const optional<T, Base1>& lhs, const optional<U, Base2>& rhs, F&& f)
{
    // A peculiar rule:
    // "lhs is considered less than rhs if, and only if, rhs contains a value and lhs does not."
    // I have a feeling I interpreted this wrong.

    /*
     * Works, just a bit arcane for only the benefit of c++11
    return lhs ?
        // If lhs and rhs both have a value, do compare.
        // If lhs has value and rhs doesn't, presume no match
        (rhs ? f() : false) :
        // If lhs and rhs have no value, that's a match
        // Otherwise it's that peculiar condition 1-7 'less than' guy
        (rhs == false ? true : less_than);
    */

    if(!lhs)
    {
        if(!rhs)    return true;

        // See "peculiar rule" above
        return less_than;
    }

    // Not seeing a specification for this condition exactly, but I think it's safe to say if
    // lhs has a value and rhs doesn't, then it's not a match
    if(!rhs)    return false;

    return f();
}

}

template <class T, class T2, class Base, class Base2>
constexpr bool operator>(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{ return internal::optional_compare(lhs, rhs, [&] { return *lhs > *rhs; }); }

template <class T, class T2, class Base, class Base2>
constexpr bool operator>=(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{ return internal::optional_compare(lhs, rhs, [&] { return *lhs >= *rhs; }); }

template <class T, class T2, class Base, class Base2>
constexpr bool operator<(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{ return internal::optional_compare<true>(lhs, rhs, [&] { return *lhs < *rhs; }); }

template <class T, class T2, class Base, class Base2>
constexpr bool operator<=(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{ return internal::optional_compare(lhs, rhs, [&] { return *lhs <= *rhs; }); }

// DEBT: Use gcc push/pop here and also filter 'U' by is_optional

#define OPERATOR_INTRINSIC_LEFT_OPT(op)     \
template <class T, class U, class Base>     \
constexpr bool operator op(const optional<T, Base>& opt, const U& value)  \
{ return opt && *opt op value; }

#define OPERATOR_INTRINSIC_RIGHT_OPT(op)    \
template <class T, class U, class Base>     \
constexpr bool operator op(const U& value, const optional<T, Base>& opt)  \
{ return opt && value op *opt; }

#define OPERATOR_INTRINSIC(op)  \
OPERATOR_INTRINSIC_LEFT_OPT(op) \
OPERATOR_INTRINSIC_RIGHT_OPT(op)

OPERATOR_INTRINSIC(>)
OPERATOR_INTRINSIC(<)
OPERATOR_INTRINSIC(>=)
OPERATOR_INTRINSIC(<=)
OPERATOR_INTRINSIC(==)

#undef OPERATOR_INTRINSIC
#undef OPERATOR_INTRINSIC_RIGHT_OP
#undef OPERATOR_INTRINSIC_LEFT_OP


}