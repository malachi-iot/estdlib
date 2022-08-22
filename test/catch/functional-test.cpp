#include <catch.hpp>

#include <estd/functional.h>

#include "test-data.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#if __has_warning("-Wunused-but-set-variable")
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

static const char* got_something = NULLPTR;

int do_something(const char* msg)
{
    got_something = msg;
    //printf("I did something: %s\n", msg);

    return -1;
}

using namespace estd;

// temporarily switching this on or off here as I build out experimental pre C++03 tuple
#ifdef FEATURE_CPP_VARIADIC


void forwarder_func(int val, estd::detail::function<void(int)> f)
{
    f(val);
}

template <class ...TArgs>
void forwarder(TArgs&&...args)
{
    forwarder_func(std::forward<TArgs>(args)...);
}

struct ContextTest
{
    int val = 0;

    int add(int v) { return val += v; }
};

template <typename T>
using fb = estd::detail::function<T, estd::detail::impl::function_virtual<T> >;


TEST_CASE("functional")
{
    SECTION("function")
    {
        SECTION("experimental")
        {
            SECTION("simplest lambda")
            {
                estd::experimental::function<int()> f = []() { return 5; };

                int val = f();

                REQUIRE(val == 5);
            }
            SECTION("ref capture lambda")
            {
                int val2 = 5;
                int val3 = 0;

                estd::experimental::function<int(int)> f = [&val2](int x) { return val2++; };

                REQUIRE(f(0) == 5);
                f = [&](int x) { ++val3; return val2++; };
                REQUIRE(f(1) == 6);
                REQUIRE(val2 == 7);
                REQUIRE(val3 == 1);
            }
            SECTION("in place execution")
            {
                int val = 0;

                estd::experimental::function<void(int)>([&](int v) { val = v; })(5);

                REQUIRE(val == 5);
            }
            SECTION("passing to another")
            {
                int val = 0;

                estd::experimental::function<void(int)> f([&](int v) { val = v; });

                SECTION("function_base")
                {
                    auto tester = [](estd::detail::function<void(int)> _f, int param)
                    {
                        _f(std::move(param));
                    };

                    tester(f, 5);

                    REQUIRE(val == 5);
                }
                SECTION("function_base perfect forwarding")
                {
                    forwarder(5, f);

                    REQUIRE(val == 5);
                }
            }
            SECTION("complex parameter lambda")
            {
                int val2 = 5;
                int val3 = 0;

                estd::experimental::function<int(int*, int)> f = [&val2](int* dest, int x) { return ++val2 + x; };

                REQUIRE(f(&val3, 1) == 7);

                int _dest = 0;

                auto f2 = estd::experimental::function<void(int*, int)>::make_inline2(
                    [&](int* dest, int x)
                {
                    *dest += x;
                });

                f2(&_dest, 1);
                REQUIRE(_dest == 1);
            }
            SECTION("make_inline")
            {
                auto i = estd::experimental::function<int(int)>::make_inline([](int x) { return x + 1; });

                REQUIRE(i.exec(1) == 2);

                estd::detail::function<int(int)> f(&i);

                REQUIRE(f.operator()(1) == 2);

                int outside_scope_value = 5;
                auto l = [&](int x) { return x + outside_scope_value; };

                estd::experimental::inline_function<decltype(l), int(int)> _if(std::move(l));
                /*
                auto _if2 =
                    estd::experimental::make_inline_function<decltype(l), int(int)>(std::move(l)); */

                REQUIRE(_if(5) == 10);

                auto _if3 = estd::experimental::function<int(int)>::make_inline2(
                    [&](int x) { return x * outside_scope_value; });

                REQUIRE(_if3(5) == 25);

                estd::detail::function<int(int)> fb1(_if3);

                REQUIRE(fb1(5) == 25);
            }
            SECTION("upcast")
            {
                auto dynamic_f = new estd::experimental::function<int(int)>([](int v)
                {
                    return v;
                });

                SECTION("initialization")
                {
                    estd::detail::function<int(int)> f(*dynamic_f);
                }
                SECTION("assignment")
                {
                    estd::detail::function<int(int)> f;

                    f = *dynamic_f;
                }

                delete dynamic_f;
            }
            SECTION("context_function")
            {
                ContextTest ctx;

                estd::experimental::context_function<int(int)>::
                    model<ContextTest, &ContextTest::add>
                    m(&ctx);

                //int sz = sizeof(m.f);
                REQUIRE(sizeof(m) == sizeof(ContextTest*) + sizeof(m.f));

                estd::experimental::context_function<int(int)> f(m);

                f(5);

                REQUIRE(ctx.val == 5);
            }
            SECTION("context_function2")
            {
                ContextTest ctx;

                estd::experimental::context_function2<
                    decltype(&ContextTest::add), &ContextTest::add> f(&ctx);

                f(5);

                REQUIRE(ctx.val == 5);
            }
            SECTION("bind")
            {
                SECTION("helper")
                {
                    // hitting issue described here:
                    // https://stackoverflow.com/questions/43560492/how-to-extract-lambdas-return-type-and-variadic-parameters-pack-back-from-gener
                    // I am certain at one time I went down the function_traits/callable_traits route.  Perhaps it's
                    // lingering on an unfinished branch somewhere?
                    //typedef estd::experimental::bind_helper<decltype([](){})> bh;
                }
                SECTION("lambda")
                {
                    int val = 0;
                    //auto b = estd::experimental::bind([&](){ val += 2; });
                }
                SECTION("to member")
                {
                    ContextTest ctx;

                    //estd::experimental::bind(&ContextTest::add, &ctx);
                }
            }
        }
    }
    SECTION("obsolete")
    {
        SECTION("basic")
        {
            // Doesn't seem to work at this point
            //estd::obsolete::function<int()> f = []() { return 5; };
        }
        SECTION("bind")
        {
            auto b = estd::obsolete::bind(do_something, "hello");

            int sz = sizeof(b);

            REQUIRE(b() == -1);

            REQUIRE(std::string(got_something) == "hello");
        }
    }
    SECTION("other impls")
    {
        SECTION("explicit")
        {
            SECTION("virtual")
            {
                typedef estd::detail::function<void(int), estd::detail::impl::function_virtual<void(
                    int)> > _fb;

                struct model : _fb::model_base
                {
                    int counter = 0;

                    virtual void _exec(int v) override
                    {
                        counter += v;
                    }
                };

                model m;
                _fb f(&m);

                f(5);

                REQUIRE(m.counter == 5);
            }
            SECTION("fnptr2")
            {
                typedef estd::detail::function<void(int), estd::detail::impl::function_fnptr2<void(
                    int)> > _fb;

                struct model : _fb::model_base
                {
                    typedef _fb::model_base base_type;

                    int counter = 0;

                    model() : base_type(&_exec)
                    {}

                    static void _exec(void* _this, int v)
                    {
                        ((model*)_this)->counter += v;
                    }
                };

                model m;
                _fb f(&m);

                f(5);

                REQUIRE(m.counter == 5);
            }
        }
        SECTION("aliased")
        {
            fb<void(int&&)> f;

            struct model : decltype(f)::model_base
            {
                int counter = 0;

                virtual void _exec(int&& v) override
                {
                    counter += v;
                }
            };

        }
    }
}

#endif

#pragma GCC diagnostic pop
