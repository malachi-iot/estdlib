#include <estd/queue.h>
#include <estd/forward_list.h>
#include <estd/array.h>
#include <estd/span.h>
#include <estd/string_view.h>
// streambuf and friends still need C++11 mainly due to ESTD_HAS_FN behavior
//#include <estd/ostream.h>
#include <estd/optional.h>

#include <stdio.h>

void test()
{
    int buf[10];

    // FIX: This actually shouldn't run, but it does
    estd::span<int> val(buf);
}


