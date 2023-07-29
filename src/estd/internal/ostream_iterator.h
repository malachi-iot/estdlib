#pragma once

#include "iosfwd.h"

namespace estd {

namespace experimental {

template <class T, class Out, class = void>
class ostream_iterator;

template <class T, class Out>
class ostream_iterator<T, Out>
{
    typedef Out ostream_type;

    ostream_type& out;

    using char_traits = typename ostream_type::traits_type;

public:
    constexpr explicit ostream_iterator(ostream_type& out) : out{out} {}

    const ostream_iterator& operator=(const T& value) const // NOLINT
    {
        out << value;
        return *this;
    }

    constexpr ostream_iterator& operator++() const { return *this; }
    constexpr ostream_iterator operator++(int) const { return *this; }  // NOLINT
    constexpr ostream_iterator operator*() const { return *this; }
};


}


}