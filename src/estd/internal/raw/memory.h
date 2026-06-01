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

}