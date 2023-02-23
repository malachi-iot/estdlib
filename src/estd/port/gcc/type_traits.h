#pragma once

// TODO: Check compiler version to ensure we indeed can support this

namespace estd {

/// The underlying type of an enum.
template<typename T>
struct underlying_type
{
    typedef __underlying_type(T) type;
};

template<typename _Tp>
struct is_trivial
    : public integral_constant<bool, __is_trivial(_Tp)>
{ };

#define FEATURE_ESTD_UNDERLYING_TYPE 1
#define FEATURE_ESTD_IS_TRIVIAL 1

}