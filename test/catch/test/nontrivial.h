#pragma once

#include <functional>

namespace estd { namespace test {

// NOTE: Specifically excludes assignment (operator =) to more deeply
// exercise consuming API
struct NonTrivial
{
    const int code_;
    const bool copied_ = false;
    const bool moved_ = false;
    const bool initialized_ = false;

    std::function<void()> on_dtor;

    explicit NonTrivial(int code) : code_{code} {}

    template <class F>
    explicit NonTrivial(int code, F&& f) :
        code_{code}, on_dtor(std::forward<F>(f))
    {}

    NonTrivial(const NonTrivial& copy_from) :
        code_{copy_from.code_}, copied_{true}, initialized_{true},
        on_dtor(copy_from.on_dtor)
    {}

    NonTrivial(NonTrivial&& move_from) noexcept :
        code_{move_from.code_}, moved_{true}, initialized_{true},
        on_dtor(std::move(move_from.on_dtor))
    {}

    ~NonTrivial()
    {
        if(on_dtor) on_dtor();
    }
};


constexpr unsigned dtor_count_1() { return 3; }
constexpr unsigned dtor_count_2() { return dtor_count_1() + 1; }



}}
