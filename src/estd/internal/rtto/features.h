#pragma once

// Occupies ROM space even when not used
#ifndef FEATURE_ESTD_RTTO_GET_METADATA
#define FEATURE_ESTD_RTTO_GET_METADATA 0
#endif

// EXPERIMENTAL - retrieve invokable function pointer (caller must know exact signature, dangerous!)
// In theory saves space for fnptr2, we can rely on just utility fnptr rather than utility+exec.
// One wonders if a vtable is better at this point though
#ifndef FEATURE_ESTD_RTTO_GET_INVOKE
#define FEATURE_ESTD_RTTO_GET_INVOKE 1
#endif
