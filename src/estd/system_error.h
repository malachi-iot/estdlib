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

#ifdef E2BIG
#define FEATURE_POSIX_ERRNO
#endif

// manual "enum class", enum itself must always be called 'values'
struct errc
{
    enum values
    {
#ifdef  FEATURE_POSIX_ERRNO
        address_family_not_supported = EAFNOSUPPORT,
        bad_address = EFAULT,
        invalid_argument = EINVAL,
        no_such_process = ESRCH,
        not_supported = ENOTSUP,
#else
        invalid_argument = EDOM + 10,
#endif
        result_out_of_range = ERANGE,
#ifdef  FEATURE_POSIX_ERRNO
        timed_out = ETIMEDOUT
#else
        timed_out = EDOM + 11
#endif
    };
};

}

typedef internal::enum_class<internal::errc> errc;

#endif

}