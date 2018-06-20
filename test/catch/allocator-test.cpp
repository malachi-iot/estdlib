#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include "mem.h"

#include "estd/allocators/handle_desc.h"
#include "estd/allocators/fixed.h"

using namespace estd;
using namespace estd::internal;

TEST_CASE("allocator tests")
{
    SECTION("fixed allocator handle descriptor")
    {
        typedef single_fixedbuf_allocator<int, 10> allocator_t;

        SECTION("inline")
        {
            handle_descriptor<allocator_t> d;

            int szof = sizeof(d);

            int size = d.size();
            //d.reallocate(5);
            int* val_array = &d.lock();
        }
        // Our unit tests always have CONSTEXPR set, but sometimes I manually disable it to test for < C++11 compatibility
#ifdef FEATURE_CPP_CONSTEXPR
        SECTION("referenced")
        {
            allocator_t allocator;
            handle_descriptor<allocator_t&> d(allocator);

            int szof = sizeof(d);

            int size = d.size();
            //d.reallocate(5);
            int* val_array = &d.lock();
        }
#endif
    }
}
