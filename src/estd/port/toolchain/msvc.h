#pragma once

#define __alignof__(T) __alignof(T)

// DEBT: Phase out ESTD_ARCH_BITNESS for more gnu-emulated LP64/ILP32
#if _WIN32 || _WIN64
#if _WIN64
#define ESTD_ARCH_BITNESS   64
#ifndef __LP64__
#define __LP64__
#endif
#else
#define ESTD_ARCH_BITNESS   32
#ifndef __ILP32__
#define __ILP32__
#endif
#endif
#endif
