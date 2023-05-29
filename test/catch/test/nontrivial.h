#pragma once

#include <functional>

namespace estd { namespace test {

struct NonTrivial
{
    const int code_;
    const bool copied_ = false;
    const bool moved_ = false;

    std::function<void()> on_dtor;

    static unsigned dtor_counter;

    explicit NonTrivial(int code) : code_{code} {}

    template <class F>
    explicit NonTrivial(int code, F&& f) :
        code_{code}, on_dtor(std::move(f))
    {}

    NonTrivial(const NonTrivial& copy_from) :
        code_{copy_from.code_}, copied_{true},
        on_dtor(copy_from.on_dtor)
    {}
    NonTrivial(NonTrivial&& move_from) :
        code_{move_from.code_}, moved_{true},
        on_dtor(std::move(move_from.on_dtor))
    {}

    ~NonTrivial()
    {
        ++dtor_counter;
        if(on_dtor) on_dtor();
    }
};


constexpr unsigned dtor_count_1() { return 3; }
constexpr unsigned dtor_count_2() { return dtor_count_1() + 1; }



}}
