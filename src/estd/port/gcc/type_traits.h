#pragma once

// TODO: Check compiler version to ensure we indeed can support this

namespace estd {

/// The underlying type of an enum.
template<typename T>
struct underlying_type
{
    typedef __underlying_type(T) type;
};

}