/*
 * @file
 */
#pragma once

#include "platform.h"

#if !defined(FEATURE_STD_SYSTEM_ERROR_ALIAS) && FEATURE_STD_SYSTEM_ERROR
#define FEATURE_STD_SYSTEM_ERROR_ALIAS 1
#endif

// 27JUL26 MB - Default to std::errc alias.  Arduino (non std) test has not happened
// in quite some time.
#if FEATURE_STD_SYSTEM_ERROR_ALIAS
#include <system_error>
namespace estd {

using std::errc;

#else

#if FEATURE_STD_CERRNO
#include <cerrno>
#else
#include <errno.h>
#endif

namespace estd {

#ifndef FEATURE_STD_FULL_ERRNO
// NOTE: Particularly usefor for VisualDSP which only implements the 3 described under "ISO C"
// portion here https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
#if     defined(FEATURE_POSIX_ERRNO)
#define FEATURE_STD_FULL_ERRNO  1
#endif
#endif


// https://en.cppreference.com/cpp/header/system_error
// https://man7.org/linux/man-pages/man3/errno.3.html
// https://www.open-std.org/jtc1/sc22/open/n4217.pdf starting at lines 7564
enum class errc
{
    none = 0,   ///< Nonstandard - mainly so that errc{} doesn't give compiler warnings

    argument_out_of_domain = EDOM,
#if     FEATURE_STD_FULL_ERRNO
    address_family_not_supported = EAFNOSUPPORT,
    address_in_use = EADDRINUSE,
    argument_list_too_long = E2BIG,
    bad_address = EFAULT,
    device_or_resource_busy = EBUSY,
    invalid_argument = EINVAL,
    no_buffer_space = ENOBUFS,
    no_lock_available = ENOLCK,
    no_message_available = ENODATA,
    no_stream_resources = ENOSR,
    no_such_file_or_directory = ENOENT,
    no_such_process = ESRCH,
    not_enough_memory = ENOMEM,
    not_supported = ENOTSUP,
    operation_canceled = ECANCELED,
    operation_in_progress = EINPROGRESS,
    operation_not_permitted = EPERM,
    operation_not_supported = EOPNOTSUPP,
    operation_would_block = EWOULDBLOCK,
#else
    invalid_argument = EDOM + 10,
    not_supported = EDOM + 20,
#endif
    result_out_of_range = ERANGE,
    resource_unavailable_try_again = EAGAIN,
#if     FEATURE_STD_FULL_ERRNO
    timed_out = ETIMEDOUT,
    value_too_large = EOVERFLOW
#else
    timed_out = EDOM + 30,
    value_too_large = EDOM + 40
#endif
};

#endif

}
