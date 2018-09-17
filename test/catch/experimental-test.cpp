#include <catch.hpp>

#include "estd/array.h"
//#include "estd/exp/buffer.h"
#include "mem.h"
#include "test-data.h"
#include <estd/string.h>
#include <estd/exp/memory_pool.h>
#include <estd/memory.h>
#include <estd/functional.h>

struct TestA {};

struct Test
{
    int a;
    float b;

    //std::string s;
    TestA& t;

    constexpr Test(int a, float b, TestA& t) :
        a(a), b(b), t(t) {}
};

TestA t;

template <class TBase>
struct provider_test : TBase
{
    typedef TBase value_provider;
    typedef typename value_provider::value_type value_type;

    template <class T>
    void do_require(const T& value)
    {
        const value_type& v = value_provider::value();

        REQUIRE(v == value);
    }

    provider_test() {}

    provider_test(int v) : value_provider (v) {}
};

int global_provider_test_value = 6;

estd::layer1::string<128> provider_string;

TEST_CASE("experimental tests")
{
    SECTION("A")
    {
        //estd::experimental::layer0
        constexpr Test test[] =
        {
            { 1,2, t },
            { 2, 3, t}
        };
        //constexpr Test test1(1, 2, t);
    }
    SECTION("accessor")
    {
        _allocator<int> a;
        int* val = a.allocate(1);

        *val = 5;

        estd::experimental::stateful_locking_accessor<_allocator<int>> acc(a, val);

        int& val2 = acc;

        REQUIRE(val2 == 5);

        a.deallocate(val, 1);
    }
    SECTION("bitness size_t deducer")
    {
        SECTION("8-bit")
        {
            typedef estd::internal::deduce_fixed_size_t<10> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 1);
        }
        {
            typedef estd::internal::deduce_fixed_size_t<100> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 1);
        }
        SECTION("16-bit")
        {
            typedef estd::internal::deduce_fixed_size_t<1000> deducer_t;
            REQUIRE(sizeof(deducer_t::size_type) == 2);
        }
    }
    SECTION("providers")
    {
        using namespace estd::experimental;

        SECTION("temporary")
        {
            provider_test<temporary_provider<int> > pt;

            REQUIRE(pt.value() == 0);

            pt.do_require(0);
        }
        SECTION("instanced")
        {
            provider_test<instance_provider<int> > pt(5);

            REQUIRE(pt.value() == 5);

            pt.do_require(5);
        }
        SECTION("global")
        {
            provider_test<global_provider<int&, global_provider_test_value> > pt;

            REQUIRE(pt.value() == 6);

            pt.do_require(6);

            provider_test<global_provider<
                    estd::layer1::string<128>&,
                            provider_string> > pt2;

            estd::layer1::string<64> f = "hi2u";

            provider_string = "hi2u";

            REQUIRE(pt2.value() == f);
            REQUIRE(pt2.value() == "hi2u");

            pt2.do_require(f);
        }
        SECTION("global")
        {
            provider_test<literal_provider<int, 7> > pt;

            REQUIRE(pt.value() == 7);

            pt.do_require(7);
        }
        SECTION("pointer from value")
        {
            provider_test<pointer_from_instance_provider<int> > pt(5);

            REQUIRE(*pt.value() == 5);
        }
    }
    SECTION("memory pool")
    {
        using namespace estd;
        using namespace estd::experimental;

        SECTION("simple integer pool")
        {
            estd::experimental::memory_pool_1<int, 10> pool;

            int* i = pool.allocate();
            REQUIRE(pool.count_free() == 9);
            int& i2 = pool.construct(3);
            REQUIRE(pool.count_free() == 8);

            pool.deallocate(i);
            REQUIRE(pool.count_free() == 9);

            REQUIRE(i2 == 3);

            pool.destroy(i2);
            REQUIRE(pool.count_free() == 10);
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
            memory_pool_1<layer1::shared_ptr<test::Dummy>, 10> pool;
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
#ifdef FEATURE_ESTD_EXP_AUTOCONSTRUCT
                REQUIRE(_p.use_count() == 1);

#else
                REQUIRE(_p.use_count() == 0);

                pool.destroy(_p);
#endif

                /*
                 * this doesn't do what you expect because what really needs to happen
                 * is a pool.construct().construct() to activate allocated shared_ptr,
                 * otherwise the layer3 initialization ends up empty.
                 * trouble continues because even if you DO construct().construct(),
                 * this code doesn't have easy access to the initial shared_ptr which
                 * one still has to call reset() on
                layer3::shared_ptr<test::Dummy> p(pool.construct());

                REQUIRE(p.use_count() == 1); */

#ifdef FEATURE_ESTD_EXP_AUTOCONSTRUCT
                int counter = 0;
                auto F2 = [&](layer3::shared_ptr<test::Dummy> p)
                {
                    counter++;
                    REQUIRE(p.use_count() == 3);
                };

                layer3::shared_ptr<test::Dummy> p(_p);

                REQUIRE(_p.use_count() == 2);

                F2(p);

                REQUIRE(counter == 1);

                //F2(_p);

                REQUIRE(p.use_count() == 2);

                _p.reset();

                REQUIRE(p.use_count() == 1);
#endif
            }

            REQUIRE(pool.count_free() == 10);
        }
    }
}
