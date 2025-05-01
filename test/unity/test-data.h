#include "../catch/test/nontrivial.h"

#if ESP_PLATFORM
#include <esp-resource.h>
#endif

namespace test {

static char span_buf[128] = "ABC";

}