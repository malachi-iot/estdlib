#pragma once

namespace estd {

// Shamelessly lifted from https://en.cppreference.com/w/cpp/memory/addressof.html
template< class T >
T* addressof(T& arg)
{
    return reinterpret_cast<T*>(
               &const_cast<char&>(
                  reinterpret_cast<const volatile char&>(arg)));
}

template<class T> const T* addressof(const T&&) = delete;

// Shamelessly adapted from https://en.cppreference.com/cpp/memory/is_sufficiently_aligned

template<std::size_t N, class T>
bool is_sufficiently_aligned(T* ptr)
{
    return ((std::uintptr_t)ptr) % N == 0;
}

}