#include <catch.hpp>

#include <estd/exp/memory/memory_pool.h>

#include "../test-data.h"
#include "../macro/push.h"

TEST_CASE("experimental: memory pool v1 tests")
{
    using namespace estd;
    using namespace estd::experimental;

    SECTION("simple integer pool")
    {
        typedef estd::experimental::memory::v1::memory_pool_1<int, 10> memory_pool_type;
        memory_pool_type pool;

        int* i = pool.allocate();
        REQUIRE(pool.count_free() == 9);
        int& i2 = pool.construct(3);
        REQUIRE(pool.count_free() == 8);

        pool.deallocate(i);
        REQUIRE(pool.count_free() == 9);

        REQUIRE(i2 == 3);

        pool.destroy(i2);
        REQUIRE(pool.count_free() == 10);

        constexpr int sz = sizeof(pool);
        constexpr int sz_item = sizeof(memory_pool_type::item);
        constexpr int sz_size_type = sizeof(memory_pool_type::size_type);

        // TODO: We actually want to autodeduce this size to uint8_t, etc. eventually
        REQUIRE(sz_size_type == sizeof(uint8_t));
        REQUIRE(sz_item == sizeof(int) + sz_size_type);
        REQUIRE(sz == sz_item * 10 + sz_size_type);
    }
    SECTION("low level access")
    {
        typedef memory::v1::memory_pool_1<int, 10> pool_type;
        pool_type pool;

        auto& item = pool.allocate_item();
        auto& item2 = pool.allocate_item();

        auto& _item2 = pool.lock(item._next);

        REQUIRE(&item2 == &_item2);

        SECTION("peering in with linked list")
        {
            // this linked list relies completely on the storage from 'pool',
            // including the intrusive portion (the 'next' handle).  The only
            // memory allocated for ext_list_type therefore should be a pointer
            // to the original storage (for lock resolution) and the current head
            pool_type::item_ext_node_traits traits(pool.node_traits().storage);
            pool_type::ext_list_type list(traits);

            list.push_front(item);
            list.push_front(item2);

            // odd, looks like my iterators are cross wired
            int sz = estd::distance(list.begin(), list.end());

            REQUIRE(sz == 2);

            pool_type::item_ext_node_traits::node_type& f = list.front();

            REQUIRE(&f == &item2);
            list.pop_front();
            REQUIRE(&list.front() == &item);
        }
    }
    SECTION("alignment testing")
    {
        struct
                //alignas(8)
                test1 { int val; };

        typedef estd::experimental::memory::v1::memory_pool_1<test1, 10> memory_pool_type;
        memory_pool_type pool;

        constexpr int sz = sizeof(pool);
        constexpr int sz_item = sizeof(memory_pool_type::item);
        constexpr int sz_size_type = sizeof(memory_pool_type::size_type);

        // TODO: We actually want to autodeduce this size to uint8_t, etc. eventually
        REQUIRE(sz_size_type == sizeof(uint8_t));

        // Not ready yet, still experimenting
        //REQUIRE(sz_item == sizeof(int*));

        // was expecting '81' but got '84'.  didn't expect alignof/alignas to affect this,
        // but that's why we experiment
        //REQUIRE(sz == sz_item * 10 + sz_size_type);
    }
    SECTION("advanced shared_ptr pool")
    {
        typedef void (*deleter_fn)(test::Dummy* to_delete, void* context);
        deleter_fn F = [](test::Dummy* to_delete, void* context)
        {

        };

        // NOTE: Can't easily get our destructor in here for shared_ptr - damn
        // I see now why they passed in their Destructor type in shared_ptr constructor
        // - so instead, memory_pool_1 has s specialized behavior when it
        // encounters layer1 shared ptrs
        memory::v1::memory_pool_1<layer1::shared_ptr<test::Dummy>, 10> pool;
        typedef typename decltype (pool)::value_type shared_ptr;

        // NOTE: pool construct will call shared_ptr constructor,
        // but shared_ptr themselves don't auto construct their
        // managed pointer
        /*
        layer1::shared_ptr<test::Dummy>& p = pool.construct();
        layer1::shared_ptr<test::Dummy>& p3 = pool.construct();
        */
#ifdef FEATURE_ESTD_EXP_AUTOCONSTRUCT
        shared_ptr& p = pool.construct(7, "hi2u");
        shared_ptr& p3 = pool.construct(8, "hi2u!");
#else
        auto& p = pool.construct();
        auto& p3 = pool.construct();
#endif

        REQUIRE(pool.count_free() == 8);

        {
            layer3::shared_ptr<test::Dummy> p2(p);

#ifdef FEATURE_ESTD_EXP_AUTOCONSTRUCT
            REQUIRE(p.use_count() == 2);
            REQUIRE(p3.use_count() == 1);
#else
            REQUIRE(p.use_count() == 0);
            REQUIRE(p3.use_count() == 0);

            p.construct(7, "hi2u");
            p3.construct(8, "hi2u!");

            REQUIRE(p.use_count() == 1);
#endif

            p2 = p;

            REQUIRE(p.use_count() == 2);

            REQUIRE(p3.use_count() == 1);

            layer3::shared_ptr<test::Dummy> p4(p3);

            REQUIRE(p3.use_count() == 2);
            REQUIRE(p4.use_count() == 2);

            p3.reset();

            // superfluous - p4 going out of scope achieves the same affect
            //p4.reset();
        }

        REQUIRE(pool.count_free() == 9);

        REQUIRE(p.use_count() == 1);

        // now that shared_ptr's own destructor is linked in to pool.destory,
        // you must never call pool.destroy manually - otherwise it will cascade
        // out through shared_ptr's destructor and call itself again (via pool.destroy_internal)
        //pool.destroy(p);
        p.reset();  // do this instead of direct pool.destroy

        REQUIRE(pool.count_free() == 10);

        SECTION("layer3 conversions")
        {
            shared_ptr& _p = pool.construct();

            // there's a slight oddness (but maybe necessary) in that a
            // freshly constructed shared_ptr has no use count, but obviously
            // counts as allocated in the pool.  this means that you have to
            // actually use the shared_ptr if you ever expect it to be freed,
            // since its pool removal is kicked off by shared_ptr's auto destruction
            // at the moment if you want to bypass that, then in this condition
            // only you can call pool.destroy
#ifndef FEATURE_ESTD_EXP_AUTOCONSTRUCT
            _p.construct();
#endif
            REQUIRE(_p.use_count() == 1);

            /*
                * this doesn't do what you expect because what really needs to happen
                * is a pool.construct().construct() to activate allocated shared_ptr,
                * otherwise the layer3 initialization ends up empty.
                * trouble continues because even if you DO construct().construct(),
                * this code doesn't have easy access to the initial shared_ptr which
                * one still has to call reset() on
            layer3::shared_ptr<test::Dummy> p(pool.construct());

            REQUIRE(p.use_count() == 1); */

            REQUIRE(pool.count_free() == 9);

            int counter = 0;
            auto F2 = [&](layer3::shared_ptr<test::Dummy> p)
            {
                counter++;
                REQUIRE(p.use_count() == 3);
            };

            layer3::shared_ptr<test::Dummy> p = _p;

            REQUIRE(_p.use_count() == 2);

            F2(p);

            REQUIRE(counter == 1);

            F2(_p);

            REQUIRE(p.use_count() == 2);

            REQUIRE(counter == 2);

            _p.reset();

            REQUIRE(p.use_count() == 1);
        }

        REQUIRE(pool.count_free() == 10);
    }
    SECTION("memory-pool specific make_shared")
    {
        memory::v1::memory_pool_1<layer1::shared_ptr<test::Dummy>, 10> pool;
        typedef typename decltype (pool)::value_type shared_ptr;

        // would be better to do this kind of in reverse, where make_shared can take any allocator,
        // including a memory pool
        shared_ptr& p = experimental::memory::v1::make_shared(pool);

        REQUIRE(p.use_count() == 1);
        REQUIRE(pool.count_free() == 9);

        p.reset();  // will auto-destroy Dummy and free from pool

        REQUIRE(pool.count_free() == 10);
    }
    SECTION("ll pool")
    {
        // almost there, just some lingering pointer vs non pointer descrepency for handling
        // '_next'
        typedef memory::v1::memory_pool_ll<int, 10> pool_type;
        intrusive_forward_list<pool_type::item> list;
        pool_type pool;

        // Skip these size checks if not in ideal scenario, and all these
        // tests are phasing out anyhow
#if ESTD_ARCH_BITNESS == 64
        unsigned sz = sizeof(pool_type::item);
        unsigned expected_sz = sizeof(int) + sizeof(void*) + 4; // extra 4 because of padding on 64-bit gnu

        REQUIRE(sz == expected_sz);

        sz = sizeof(pool);
        REQUIRE(sz == (expected_sz * 10) + sizeof(void*));
#endif

        int* val1 = pool.allocate();

        REQUIRE(pool.count_free() == 9);

        *val1 = 123;

        int& val2 = pool.construct(456);

        REQUIRE(pool.count_free() == 8);

        REQUIRE(*val1 == 123);
        REQUIRE(val2 == 456);

        pool_type::item& item = pool.allocate_item();

        REQUIRE(pool.count_free() == 7);

        // NOTE: almost works - as expected, traits are different for this particular
        // node type.  I think we can specialize here
        //list.push_front(item);
    }
}

#include "../macro/pop.h"
