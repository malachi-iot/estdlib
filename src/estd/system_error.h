/*
 * @file
 */
#pragma once

#include "internal/platform.h"

#ifdef FEATURE_STD_SYSTEM_ERROR
#include <system_error>
namespace estd {

typedef std::errc errc;

#else
namespace estd {

namespace internal {

struct enum_helper
{
protected:
    int value;

public:
    explicit enum_helper(int value) : value(value) {}
};
}

// UNTESTED
class errc : public internal::enum_helper
{
public:
    explicit errc(int value) :
        internal::enum_helper(value) {}
    
    enum values
    {
        address_family_not_supported = EAFNOSUPPORT,
        invalid_argument = EINVAL,
        result_out_of_range = ERANGE
    };

    bool operator == (values compare_to) const
    {
        return value == compare_to;
    }

    bool operator != (values compare_to) const
    {
        return value != compare_to;
    }
};

#endif

}