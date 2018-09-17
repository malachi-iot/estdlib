#include <catch.hpp>

#include <estd/allocators/fixed.h>
#include "estd/memory.h"
#include "estd/exp/tmr.h"
#include <estd/forward_list.h>

#include "mem.h"
#include "test-data.h"

#ifdef UNUSED
// https://stackoverflow.com/questions/621616/c-what-is-the-size-of-an-object-of-an-empty-class
// non-standard, but seemingly reliable way to ensure 0-length empty
// struct
struct empty
{
    char NO_DATA[0];
};
#endif

using namespace estd;

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
    SECTION("scoped_allocator_adaptor")
    {
        SECTION("basic")
        {
            experimental::scoped_allocator_adaptor<estd::layer1::allocator<int, 512> > a;

            a.allocate(15);
        }
        SECTION("nested")
        {
            experimental::scoped_allocator_adaptor<
                    estd::layer1::allocator<int, 512>,
                    estd::layer1::allocator<int, 256>> a;

            a.allocate(15);
            a.inner_allocator().allocate(20);

            typedef decltype(a) a_type;

            a_type::inner_allocator_type::outer_allocator_type test;

            REQUIRE(test.max_size() == 256);
            test.allocate(15);
        }
    }
    SECTION("experimental")
    {
        SECTION("shared_ptr [old]")
        {
            int val;
            experimental::shared_ptr<int> sp(&val);
        }
        SECTION("shared_ptr")
        {
            SECTION("layer1")
            {
                layer1::shared_ptr<test::Dummy> sp;

                // not yet constructed at this point, though
                // it is allocated
                REQUIRE(sp.use_count() == 0);

                sp->val1 = 10;

                REQUIRE(sp.provided().val1 == 10);

                sp.construct(11, "hi2u");

                REQUIRE(sp.use_count() == 1);

                REQUIRE(sp->val1 == 11);
            }
            SECTION("layer2 + 3: basic usage")
            {
                auto f = [](int*) {};
                int val = 5;

                layer2::shared_ptr<int, decltype(f)> sp(&val, f);
                REQUIRE(sp.use_count() == 1);
                layer3::shared_ptr<int> sp2(sp);
                REQUIRE(sp.use_count() == 2);
                layer3::shared_ptr<int> sp3(sp2);
                REQUIRE(sp.use_count() == 3);

                sp.reset();
                // can't use use_count() here because it's 0 once we decouple
                REQUIRE(sp.value().shared_count == 2);
                sp2.reset();
                REQUIRE(sp.value().shared_count == 1);
            }
            SECTION("layer2: using a more complex type")
            {
                // experimenting with ensuring alignment/casting is proper.  the casting
                // is needed so that dummy's destructor is not called twice
                alignas(alignof(test::Dummy)) uint8_t buffer[sizeof(test::Dummy)];
                test::Dummy* dummy = reinterpret_cast<test::Dummy*>(buffer);
                new (dummy) test::Dummy();

                layer2::shared_ptr<test::Dummy> sp(dummy);

                sp->val1 = 5;

                REQUIRE(sp.use_count() == 1);
                REQUIRE(sp->val1 == 5);
            }
            SECTION("pre-c++11-style")
            {
                static bool deleter_ran = false;
                typedef void (*deleter)(test::Dummy* to_destruct);
                test::Dummy* dummy = reinterpret_cast<test::Dummy*>(malloc(sizeof(test::Dummy)));
                deleter F = [](test::Dummy* to_delete)
                {
                    REQUIRE(to_delete->val1 == 3);
                    deleter_ran = true;
                    to_delete->~Dummy();
                    free(to_delete);
                };
                {
                    layer2::shared_ptr<test::Dummy, deleter> sp(dummy, F);

                    sp->val1 = 3;

                    REQUIRE(sp.use_count() == 1);
                }
                REQUIRE(deleter_ran);
            }
            SECTION("more pre-c++-11 style")
            {
                typedef void (*deleter)(test::Dummy* to_destruct, int& context);
                deleter F = [](test::Dummy* to_delete, int& context)
                {
                    to_delete->~Dummy();
                    //free(to_delete);
                };
                int context;
                //test::Dummy* dummy = reinterpret_cast<test::Dummy*>(malloc(sizeof(test::Dummy)));

                //layer1::shared_ptr<test::Dummy, deleter>(F, context);
            }
        }
    }
}
