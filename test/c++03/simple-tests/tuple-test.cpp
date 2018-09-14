#include <estd/internal/platform.h>

#if defined(FEATURE_CPP_VARIADIC)
#include <estd/tuple.h>

void test_tuple()
{
    estd::tuple<int, int> val(1, 2);

    int v = estd::get<0>(val);

    v = estd::get<1>(val);
}
#endif

