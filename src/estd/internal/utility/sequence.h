#pragma once

#include "../variadic/integer_sequence.h"
#include "../raw/cstddef.h"

namespace estd {

#ifdef __cpp_variadic_templates
// adapted from https://gist.github.com/ntessore/dc17769676fb3c6daa1f
template<typename T, estd::size_t N, T... Is>
struct make_integer_sequence : make_integer_sequence<T, N-1, N-1, Is...> {};

template<typename T, T... Is>
struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

// 26OCT24 MB Adapted from above technique
template<typename T, std::size_t NN, std::size_t N = NN, T... Is>
struct make_reverse_integer_sequence : make_reverse_integer_sequence<T, NN, N-1, NN-N, Is...> {};

template<typename T, std::size_t NN, T... Is>
struct make_reverse_integer_sequence<T, NN, 0, Is...> : estd::integer_sequence<T, Is...> {};

template<std::size_t N>
using make_reverse_index_sequence = make_reverse_integer_sequence<std::size_t, N>;

#endif

}