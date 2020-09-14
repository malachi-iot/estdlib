/*
 * @file
 */
#pragma once

#include "internal/platform.h"
#include "internal/enum.h"

// Not actually using std:: one yet, so as to dogfood our custom version
#ifdef FEATURE_STD_SYSTEM_ERROR
#include <system_error>
namespace estd {

typedef std::errc errc;

#else

#include <errno.h>

namespace estd {

namespace internal {


// manual "enum class", enum itself must always be called 'values'
struct errc
{
    enum values
    {
#if     _POSIX_C_SOURCE >= 199606L
        address_family_not_supported = EAFNOSUPPORT,
        bad_address = EFAULT,
        invalid_argument = EINVAL,
        no_such_process = ESRCH,
        not_supported = ENOTSUP,
#else
        invalid_argument = EDOM + 10,
        not_supported = EDOM + 20,
#endif
        result_out_of_range = ERANGE,
#if     _POSIX_C_SOURCE >= 199606L
        timed_out = ETIMEDOUT
#else
        timed_out = EDOM + 30
#endif
    };
};

}

typedef internal::enum_class<internal::errc> errc;

#endif

}