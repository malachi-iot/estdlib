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

#ifndef FEATURE_STD_FULL_ERRNO
// According to https://opensource.apple.com/source/xnu/xnu-4570.41.2/bsd/sys/cdefs.h.auto.html
// llvm/clang only defines _POSIX_C_SOURCE when one wishes to restrict API to only that
// standard, which seems a little at odds with the GNU treatment
#if     (_POSIX_C_SOURCE >= 199606L) || defined(_LIBCPP_CLANG_VER)
#define FEATURE_STD_FULL_ERRNO  1
#endif
#endif

// manual "enum class", enum itself must always be called 'values'
struct errc
{
    enum values
    {
#if     FEATURE_STD_FULL_ERRNO
        address_family_not_supported = EAFNOSUPPORT,
        bad_address = EFAULT,
        device_or_resource_busy = EBUSY,
        invalid_argument = EINVAL,
        no_such_process = ESRCH,
        not_enough_memory = ENOMEM,
        not_supported = ENOTSUP,
#else
        invalid_argument = EDOM + 10,
        not_supported = EDOM + 20,
#endif
        result_out_of_range = ERANGE,
#if     FEATURE_STD_FULL_ERRNO
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