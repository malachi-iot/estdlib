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

        handle_descriptor<allocator_t> d;

        int size = d.size();
        d.reallocate(5);
        int* val_array = &d.lock();
    }
}
