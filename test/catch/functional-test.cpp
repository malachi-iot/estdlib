#include <catch.hpp>

#include <estd/functional.h>

#include "test-data.h"
#include "test-functional.h"

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

template <typename F>
void do_something_inspector(F&&)
{
    typedef estd::experimental::function_traits<F> traits;

    REQUIRE(estd::is_same<typename traits::result_type, int>::value);
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

    // NOTE: Cannot name 'add' since estd::experimental::context_function is unable to resolve
    // overloads
    void add2() { val += 7; }

    static int add3(int v, ContextTest* c)
    {
        return c->add(v);
    }
};

template <typename T>
using fb = estd::detail::function<T, estd::detail::impl::function_virtual<T> >;

template <typename F>
struct ProvidedTest1;

template <typename TResult, typename ...TArgs>
struct ProvidedTest1<TResult(TArgs...)>
{
    typedef TResult (test)(TArgs...);

    static TResult test2(TArgs...) { return TResult(); }
};

template <typename F, typename TDummy>
struct ProvidedTest2;

template <typename TResult, typename ...TArgs, class TDummy>
struct ProvidedTest2<TResult(TArgs...), TDummy>
{
    TDummy value2;

    typedef TResult (test)(TArgs...);

    static TResult test2(TArgs...) { return TResult(); }
};


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
            SECTION("make_inline (deprecated)")
            {
                auto i = estd::experimental::function<int(int)>::make_model([](int x) { return x + 1; });

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

                estd::experimental::context_function<
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
            SECTION("fnptr1 (default)")
            {
                //estd::detail::impl::function_fnptr2<void(int)>::
                int value = 0;

                auto m = estd::internal::make_model<void(int)>(
                    [&](int v) { value += v; });

                estd::detail::function<void(int)> f(&m);

                f(5);

                REQUIRE(value == 5);
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
    SECTION("impl")
    {
        typedef estd::detail::function<int(int)> fn1_type;

        ContextTest context;
        internal::impl::function_context_provider<int(int)>::model<ContextTest, &ContextTest::add> m(&context);
        fn1_type f(&m);

        f(5);

        REQUIRE(context.val == 5);

        typedef fn1_type::imbue<ProvidedTest1> p1;

        p1::test2(5);

        //p1::test2(5);

        internal::impl::function_context_provider<fn1_type>::model<ContextTest, &ContextTest::add>
            m3(&context);

        //internal::impl::method_model<int(int), ContextTest, &ContextTest::add> m4(&context);
        internal::impl::method_type<int(int), ContextTest> m4;
        internal::impl::method_model<int(int), ContextTest, &ContextTest::add> m5(&context);

        // Doesn't play nice, presumably because TArgs2... doesn't handle the ContextTest::add part well
        //fn1_type::imbue<internal::impl::method_model, ContextTest, &ContextTest::add> m6(&context);

        detail::function<int(int)> f_m5(&m5);

        f_m5(2);

        REQUIRE(context.val == 7);

        SECTION("provided")
        {
            // Doing 'model' using this 'provider' technique in hopes of reducing verbosity
            // However, so far it doesn't really seem to be much help
            typedef fn1_type::imbue<internal::impl::function_context_provider>::
            model<ContextTest, &ContextTest::add> p2;

            p2 m2(&context);

            typedef fn1_type::imbue<ProvidedTest2, float> type1;

            type1 p3;

            p3.test2(5);
            REQUIRE(estd::is_same<decltype(type1::value2), float>::value);
        }
    }
    SECTION("function_traits")
    {
        using namespace estd::experimental;

        SECTION("direct")
        {
            typedef function_traits<decltype(do_something)> traits;

            REQUIRE(estd::is_same<traits::arg_t<0>, const char*>::value);
            REQUIRE(estd::is_same<traits::result_type, int>::value);
        }
        SECTION("functor")
        {
            do_something_inspector(do_something);
        }
        SECTION("lambda")
        {
            auto l = [](int v) { return v + 5; };

            typedef function_traits<decltype(l)> traits;

            // Not ready
            //REQUIRE(estd::is_same<traits::resullt_type, int>::value);
        }
    }
    SECTION("thisify")
    {
        ContextTest ctx;

        SECTION("int(int)")
        {
            estd::internal::thisify_function<int(int)>::
                model<ContextTest, &ContextTest::add> m2(&ctx);

            estd::internal::impl::method_model<int(int),
                ContextTest, &ContextTest::add>
                m(&ctx);

            //int sz = sizeof(m.f);
            REQUIRE(sizeof(m) == sizeof(ContextTest*) + sizeof(m.f));

            estd::detail::function<int(int)> f(&m);

            f(5);

            REQUIRE(ctx.val == 5);

            f = &m2;

            REQUIRE(f);

            f(2);

            REQUIRE(ctx.val == 7);

            f = nullptr_t{};

            REQUIRE(!f);
        }
        SECTION("void(void)")
        {
            estd::internal::impl::method_model<void(void),
                ContextTest, &ContextTest::add2>
                m(&ctx);

            // NOTE: In this case we actually are using 'm' to copy
            // into f and technically m storage is not required
            estd::internal::thisify_function<void(void)> f(m);

            f();

            REQUIRE(ctx.val == 7);
        }
        SECTION("tag/indicator")
        {
            // TODO: Try passing in a tag or an actual function pointer which
            // is immediately tossed out
        }
    }
    SECTION("contextify")
    {
        ContextTest c;
        estd::internal::contextify_function<int(int)>::model<ContextTest*, &ContextTest::add3> m(&c);
        estd::detail::function<int(int)> f(&m);

        f(5);

        REQUIRE(c.val == 5);
    }
    SECTION("static_function")
    {
        int value = 0;

        // As the name implies, I am not convinced this will play nice when used
        // inside a method - how is one static F&&/model going to differentiate
        // between multiple 'this' pointers?  Then again if the storage is a pointer
        // to the this pointer, that may be different.  We definitely don't expect
        // concurrency/multi thread to behave well here.
        estd::experimental::static_function<int(int)> f([&](int v)
        {
            return value += v;
        });

        f(5);

        REQUIRE(value == 5);
    }
    SECTION("copyable")
    {
        // Q: Can we trivially copy closure types (lambda objects) around?
        // https://en.cppreference.com/w/cpp/language/lambda
        // https://baites.github.io/computer-science/idioms/closure-series/2017/12/11/are-closures-trivial.html
        // https://stackoverflow.com/questions/32986193/when-is-a-lambda-trivial
        // https://gcc.gnu.org/legacy-ml/gcc-patches/2014-10/msg01007.html
        // TLDR; Not really, but we can copy via their copy constructor (but no assignment operator)

        // Q: Can we memcpy closure types?
        // https://github.com/habanero-rice/hclib/issues/12
        // A: Since they aren't POD/trivial, not officially.

        int val = 0;
        int val2 = 0;
        //int val3 = 0;

        typedef estd::detail::function<void()> function_type;

        auto m = function_type::make_model([&]
        {
            ++val;
            //val3 = val2 + 1;
        });

        auto m2 = function_type::make_model([=]
        {

        });

        SECTION("core")
        {

            m.exec();

            auto f = m.f;

            int sz = sizeof(f);

            REQUIRE(&f != &m.f);
            REQUIRE(val == 1);

            f();

            REQUIRE(val == 2);

            estd::detail::impl::function_fnptr1<void()>::
                    copyable_model<sizeof(f)>
                    copyable_m(f);

            estd::detail::impl::function_fnptr1<void()>::
                copyable_model<sizeof(m2.f)>
                copyable_m2(m2);

            copyable_m._exec();
            copyable_m2._exec();    // NOTE: Doesn't do anything at the moment

            REQUIRE(val == 3);

            estd::byte raw[256];

            auto c2 = new (raw) estd::detail::impl::function_fnptr1<void()>::
                    copyable_model<>(copyable_m);//, sizeof(copyable_m));

            c2->_exec();

            REQUIRE(val == 4);

            //copyable_m.helper(copyable_m);
        }
        SECTION("list")
        {
            FunctionList l;

            l.list.push_back([&]{ ++val2; });

            l.invoke();

            REQUIRE(val2 == 1);

            l.list.push_back([&]{ ++val2; });

            l.invoke();

            REQUIRE(val2 == 3);
        }
    }
}

#endif

#pragma GCC diagnostic pop
