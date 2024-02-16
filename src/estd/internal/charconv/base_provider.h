#pragma once

namespace estd { namespace internal {

template <int base_ = -1>
struct base_provider
{
    static CONSTEXPR unsigned base = (unsigned) base_;

    ESTD_CPP_CONSTEXPR_RET unsigned operator()() const { return base; }
};

template <>
struct base_provider<-1>
{
    typedef unsigned type;

    const type base;

    ESTD_CPP_CONSTEXPR_RET EXPLICIT base_provider(type base) : base{base} {}
    ESTD_CPP_CONSTEXPR_RET EXPLICIT base_provider(int base) : base{(type)base} {}

    ESTD_CPP_CONSTEXPR_RET unsigned operator()() const { return base; }
};



}}