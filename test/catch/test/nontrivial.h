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
    explicit NonTrivial(const NonTrivial& copy_from) :
        code_{copy_from.code_}, copied_{true}
    {}

    ~NonTrivial()
    {
        ++dtor_counter;
    }
};


constexpr unsigned dtor_count_1() { return 3; }
constexpr unsigned dtor_count_2() { return dtor_count_1() + 1; }



}}
