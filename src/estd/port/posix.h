// DEBT: Pretty sure filtering Chrono, etc. by POSIX isn't right.  Feels more like a stdlib++ type filter
// IOS is an exception, we cascade out to fopen etc. for that one
#if defined(ESTD_OS_UNIX) || defined(ESTD_OS_MACOS)
#define ESTD_POSIX
#define FEATURE_POSIX_IOS
#define FEATURE_POSIX_THREAD
#define FEATURE_POSIX_CHRONO
#define FEATURE_POSIX_ERRNO
#elif defined(__MINGW32__)
#define FEATURE_POSIX_IOS
#define FEATURE_POSIX_THREAD
#define FEATURE_POSIX_CHRONO
#define FEATURE_POSIX_ERRNO
#elif _POSIX_C_SOURCE >= 199606L
#define FEATURE_POSIX_IOS
#define FEATURE_POSIX_ERRNO
#elif defined(_LIBCPP_CLANG_VER)
// According to https://opensource.apple.com/source/xnu/xnu-4570.41.2/bsd/sys/cdefs.h.auto.html
// llvm/clang only defines _POSIX_C_SOURCE when one wishes to restrict API to only that
// standard, which seems a little at odds with the GNU treatment
// NOTE: Particularly usefor for VisualDSP which only implements the 3 described under "ISO C"
// portion here https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
// DEBT: Pretty sure we never get here, just copy/pasting from our system_error.h
#define FEATURE_POSIX_IOS
#define FEATURE_POSIX_THREAD
#define FEATURE_POSIX_CHRONO
#define FEATURE_POSIX_ERRNO
#endif
