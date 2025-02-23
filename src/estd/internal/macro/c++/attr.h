#pragma once

// NOTE: Not using feature test has_macro since I've seen that glitch
// out in llvm IIRC
#if __cplusplus >= 201703L
#define ESTD_CPP_ATTR_FALLTHROUGH   [[fallthrough]]
#define ESTD_CPP_ATTR_NODISCARD     [[nodiscard]]
#else
#define ESTD_CPP_ATTR_FALLTHROUGH
#define ESTD_CPP_ATTR_NODISCARD
#endif
