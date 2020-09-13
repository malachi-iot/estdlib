/*
 * @file
 */
#pragma once

#include "internal/platform.h"

// Not actually using yet, just to dogfood our custom version
#ifdef FEATURE_STD_SYSTEM_ERROR
#include <system_error>
namespace estd {

typedef std::errc errc;

#else
namespace estd {

namespace internal {

// To work around < C++11's lack of "enum class"
// TEnumClass must always contain an enum called 'values'
// TODO: Move this enum_helper elsewhere for easier discovery
template <class TEnumClass, class TValue = int>
struct enum_helper : TEnumClass
{
protected:
    TValue value;

public:
    explicit enum_helper(TValue value) : value(value) {}

    typedef typename TEnumClass::values values;

    bool operator == (values compare_to) const
    {
        return value == compare_to;
    }

    bool operator != (values compare_to) const
    {
        return value != compare_to;
    }
};

// manual "enum class", enum itself must always be called 'values'
struct errc_enum
{
    enum values
    {
        address_family_not_supported = EAFNOSUPPORT,
        invalid_argument = EINVAL,
        result_out_of_range = ERANGE
    };
};

}

class errc : public internal::enum_helper<internal::errc_enum>
{
public:
    explicit errc(int value) :
        internal::enum_helper<internal::errc_enum>(value) {}
    
};

#endif

}