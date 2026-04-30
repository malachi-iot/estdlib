#pragma once

#include "../platform.h"

#include "enum.h"

namespace estd { namespace internal {

constexpr int copy_to(rtto_modes::utility_type u, const void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::COPY, const_cast<void*>(src), sz, dest);
}

constexpr int move_to(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE, src, sz, dest);
}

/// Moves src -> dest then destroys src
constexpr int move_to_and_destroy(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE_AND_DESTROY, src, sz, dest);
}

inline ESTD_CPP_CONSTEXPR(14) void destroy(rtto_modes::utility_type u, void* src)
{
    u(rtto_modes::DELETE, src, 0, nullptr);
}

constexpr int size(const rtto_modes::utility_type u)
{
    return u(rtto_modes::SIZE, nullptr, 0, nullptr);
}

}}
