#pragma once

// Exceptions are frequently off in embedded environments.  For scenarios
// which usually require an exception, we can either:
// 1) fall back to a raw abort operation
// 2) ignore it completely, implying undefined behavior (at best)
// Default of this flag is to '1' which is abort fall back.  '0'
// switches that off in which case we ignore it completely.
// Code only offers this option if operation is preventable by other means
// such as checking a flag first
#ifndef FEATURE_ESTD_EXCEPTION_ABORT
#define FEATURE_ESTD_EXCEPTION_ABORT 1
#endif
