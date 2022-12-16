#define ESTD_SDK_IDF

// TODO: Phase these out since esp-idf has its own flavor now
// https://github.com/espressif/esp-idf/blob/v4.0/components/esp_common/include/esp_idf_version.h
// NOTE: Our flavor has a minor advantage that we deduce version at build time, before compile
// time
// DEBT: This is probably actually also done by esp-idf as well, as per
// https://docs.espressif.com/projects/esp-idf/en/v4.4.3/esp32/api-guides/build-system.html
// which indicates the presence of IDF_VERSION_* at build time

// Until we phase this out, dogfood ESTD_BUILD_SEMVER into here
#include "version.h"

// now, makefile magic (version_finder.mk, version_finder.cmake) populates critical values
// for flash data , this is interesting https://github.com/espressif/ESP8266_RTOS_SDK/blob/0d718b24b35481eb5614a1bc9481bbb81273b6b0/components/esp8266/include/esp_attr.h
// https://richard.burtons.org/2015/07/08/accessing-byte-data-stored-on-flash/
// couldn't find explicit copy mechanisms tailored to FLASH/alignment.  Maybe it's
// "automagic" but pretty sure gonna have to go hands-on for some of our string operations

#define ESTD_BUILD_IDF_VER(major, minor, patch, suffix) \
    ((ESTD_BUILD_SEMVER(major, minor, patch) * 10000) + suffix)
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
#define ESTD_IDF_VER_3_1_6      ESTD_BUILD_IDF_VER(3, 1, 6, 0)
#define ESTD_IDF_VER_3_2_0      ESTD_BUILD_IDF_VER(3, 2, 0, 0)
#define ESTD_IDF_VER_3_3_0      ESTD_BUILD_IDF_VER(3, 3, 0, 0)
#define ESTD_IDF_VER_3_3_1      ESTD_BUILD_IDF_VER(3, 3, 1, 0)
#define ESTD_IDF_VER_4_0_0      ESTD_BUILD_IDF_VER(4, 0, 0, 0)
#define ESTD_IDF_VER_4_4_3      ESTD_BUILD_IDF_VER(4, 4, 3, 0)
#define ESTD_IDF_VER_5_0_0      ESTD_BUILD_IDF_VER(5, 0, 0, 0)

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
