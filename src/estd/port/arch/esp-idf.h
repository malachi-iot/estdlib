// NOTE: None of this works.  Want a way to determine IDF version and react to it because
// espressif makes a lot of breaking changes
#ifdef EXPERIMENTAL

#define IDF_VER_2_0_0_444 "v2.0.0-444-g0d718b2"

// compares two strings in compile time constant fashion
constexpr int c_strcmp( char const* lhs, char const* rhs )
{
    return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
        :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
        : c_strcmp( lhs+1, rhs+1 );
}

#if (0 == c_strcmp( IDF_VER, IDF_VER_2_0_0_444 ))
# define ESTD_IDF_VER_MAJOR 2
# define ESTD_IDF_VER_MINOR 0
# define ESTD_IDF_VER_SUFFIX 444
#elif IDF_VER == "v2.0.0-644-g765754f"
# define ESTD_IDF_VER_MAJOR 2
# define ESTD_IDF_VER_MINOR 0
# define ESTD_IDF_VER_SUFFIX 644
#endif

#endif

// now, makefile magic (version_finder.mk) populates critical values
// for flash data , this is interesting https://github.com/espressif/ESP8266_RTOS_SDK/blob/0d718b24b35481eb5614a1bc9481bbb81273b6b0/components/esp8266/include/esp_attr.h
// https://richard.burtons.org/2015/07/08/accessing-byte-data-stored-on-flash/
// couldn't find explicit copy mechanisms tailored to FLASH/alignment.  Maybe it's
// "automagic" but pretty sure gonna have to go hands-on for some of our string operations

// under this approach, version 1.4.1 (for example) would be:
// 10401
// so we're giving each decimal place 100 to work with
#define ESTD_BUILD_IDF_SHORT_VER(major, minor, patch)  ((major * 10000) + (minor * 100) + patch)
#define ESTD_BUILD_IDF_VER(major, minor, patch, suffix) \
    ((ESTD_BUILD_IDF_SHORT_VER(major, minor, patch) * 10000) + suffix)
//#define ESTD_IDF_SHORT_VER    (ESTD_IDF_VER_MAJOR * 10000) + (ESTD_IDF_VER_MINOR * 100) + ESTD_IDF_VER_PATCH
//#define ESTD_IDF_VER   ((ESTD_IDF_SHORT_VER * 10000) + ESTD_IDF_VER_SUFFIX)
#define ESTD_IDF_VER    ESTD_BUILD_IDF_VER(ESTD_IDF_VER_MAJOR, ESTD_IDF_VER_MINOR, ESTD_IDF_VER_PATCH, ESTD_IDF_VER_SUFFIX)

// noteworthy ESP8266 versions
#define ESTD_IDF_VER_2_0_0_444  ESTD_BUILD_IDF_VER(2, 0, 0, 444)
#define ESTD_IDF_VER_2_0_0_644  ESTD_BUILD_IDF_VER(2, 0, 0, 644)
#define ESTD_IDF_VER_2_0_0_740  ESTD_BUILD_IDF_VER(2, 0, 0, 740)

// release versions
#define ESTD_IDF_VER_3_0_0      ESTD_BUILD_IDF_VER(3, 0, 0, 0)
#define ESTD_IDF_VER_3_0_3      ESTD_BUILD_IDF_VER(3, 0, 3, 0)
#define ESTD_IDF_VER_3_1_0      ESTD_BUILD_IDF_VER(3, 1, 0, 0)

// current ESP32 versions that I'm working with
#define ESTD_IDF_VER_3_1_0_1319 ESTD_BUILD_IDF_VER(3, 1, 0, 1319)
#define ESTD_IDF_VER_3_2_0_209  ESTD_BUILD_IDF_VER(3, 2, 0, 209)
#define ESTD_IDF_VER_3_2_0_708  ESTD_BUILD_IDF_VER(3, 2, 0, 708)

// since C preprocessor is terrible at string comparisons, we make these
// numeric
#define ESTD_IDF_VER_TAG_UNKNOWN    -1
#define ESTD_IDF_VER_TAG_NONE       0
#define ESTD_IDF_VER_TAG_DEV        1
#define ESTD_IDF_VER_TAG_RC1        2
#define ESTD_IDF_VER_TAG_RC2        3
#define ESTD_IDF_VER_TAG_RC         4
#define ESTD_IDF_VER_TAG_BETA1      5
