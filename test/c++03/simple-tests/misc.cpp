#include <estd/queue.h>
#include <estd/forward_list.h>
#include <estd/array.h>
#include <estd/span.h>
#include <estd/string_view.h>
#include <estd/ostream.h>

void test()
{
    int buf[10];

    // FIX: This actually shouldn't run, but it does
    estd::span<int> val(buf);
}


void test2()
{
    estd::layer1::string<16> s = "hello";

    char* d = s.data();
}
