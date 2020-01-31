#pragma once

#define __alignof__(T) __alignof(T)

#if _WIN32 || _WIN64
#if _WIN64
#define ESTD_ARCH_BITNESS   64
#else
#define ESTD_ARCH_BITNESS   32
#endif
#endif
