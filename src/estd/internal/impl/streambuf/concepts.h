#pragma once

#if __cpp_concepts

namespace estd { namespace internal {

namespace impl {

template <class T>
concept Streambuf = requires(T s)
{
    typename T::traits_type;
};

}

}}

#endif