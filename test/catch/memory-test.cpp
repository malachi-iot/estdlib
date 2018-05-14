#include <catch.hpp>

#include "estd/memory.h"
#include "mem.h"

// https://stackoverflow.com/questions/621616/c-what-is-the-size-of-an-object-of-an-empty-class
// non-standard, but seemingly reliable way to ensure 0-length empty
// struct
struct empty
{
    char NO_DATA[0];
};

TEST_CASE("memory.h tests")
{
    SECTION("A")
    {
#ifdef UNUSED
        estd::experimental::unique_handle<int, _allocator> uh;
        //auto s = sizeof(empty);
#endif
    }
    SECTION("rebind")
    {
        estd::experimental_std_allocator<int> a;

        float* f = a.rebind_experimental<float>().allocate(10);

        a.rebind_experimental<float>().deallocate(f, 10);
    }
}
