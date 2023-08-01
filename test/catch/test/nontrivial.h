#pragma once

#include <estd/tuple.h>

#if FEATURE_STD_FUNCTIONAL
#include <functional>
#else
#include <estd/utility.h>
#endif

namespace estd { namespace test {

#if __cplusplus >= 201103L
// NOTE: Specifically excludes assignment (operator =) to more deeply
// exercise consuming API
struct NonTrivial
{
    const int code_;
    const bool copied_ = false;
    const bool moved_ = false;
    const bool initialized_ = false;
    bool destroyed_ = false;
    bool moved_from_ = false;

#if FEATURE_STD_FUNCTIONAL
    std::function<void()> on_dtor;
#else
    bool on_dtor = false;   // dummy, for environments without std::function
#endif

    explicit NonTrivial(int code) :
        code_{code}, initialized_{true}
    {}

    template <class F>
    explicit NonTrivial(int code, F&& f) :
        code_{code}, initialized_{true}, on_dtor(std::forward<F>(f))
    {}

    template <class F>
    explicit NonTrivial(tuple<int, F> i) :
        NonTrivial(estd::get<0>(i), estd::get<1>(i))
    {}

    NonTrivial(const NonTrivial& copy_from) :
        code_{copy_from.code_}, copied_{true}, initialized_{true},
        on_dtor(copy_from.on_dtor)
    {}

    NonTrivial(NonTrivial&& move_from) noexcept :
        code_{move_from.code_}, moved_{true}, initialized_{true},
        on_dtor(std::move(move_from.on_dtor))
    {
        move_from.moved_from_ = true;
    }

    ~NonTrivial()
    {
        destroyed_ = true;
#if FEATURE_STD_FUNCTIONAL
        if(on_dtor) on_dtor();
#endif
    }
};


constexpr unsigned dtor_count_1() { return 3; }
constexpr unsigned dtor_count_2() { return dtor_count_1() + 1; }
#endif



}}
