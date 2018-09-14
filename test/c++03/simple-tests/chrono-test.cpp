// very difficult to get C++98 compliant
#include <estd/internal/platform.h>

#if defined(FEATURE_CPP_VARIADIC)
#include <estd/chrono.h>

void test_chrono()
{
    estd::chrono::steady_clock c;

    estd::chrono::steady_clock::time_point n = c.now();

    //n.count();
}
#endif
