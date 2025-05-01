#include "../catch/test/nontrivial.h"

#if ESP_PLATFORM
#include <esp-resource.h>
#endif

namespace test {

ESTD_CPP_ATTR_MAYBE_UNUSED
static char span_buf[128] = "ABC";

}