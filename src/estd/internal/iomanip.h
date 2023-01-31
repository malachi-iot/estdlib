#pragma once

#include "platform.h"

namespace estd { namespace internal {

template <class TChar>
struct setfill
{
    const TChar c;

    ESTD_CPP_CONSTEXPR_RET setfill(TChar c) : c(c) {}
};


struct setw
{
    const unsigned width;

    ESTD_CPP_CONSTEXPR_RET setw(unsigned width) : width(width) {}
};

}}