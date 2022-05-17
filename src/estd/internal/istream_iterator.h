#pragma once

#include "iosfwd.h"
#include "../cstddef.h"

namespace estd { namespace experimental {

template <class T, class TStream>
class istream_iterator
{
public:
    typedef TStream istream_type;
    typedef typename istream_type::char_type char_type;
    typedef typename istream_type::traits_type traits_type;
    typedef std::ptrdiff_t difference_type;
    typedef T value_type;

private:
    typedef typename traits_type::int_type int_type;
    istream_type* stream;

    bool end() const { return stream == NULLPTR; }

public:
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    istream_iterator() : stream(NULLPTR)
    {}

    istream_iterator(istream_type& stream) : stream(&stream)
    {

    }

    istream_iterator& operator++()
    {
        if(!end())
        {
            // Prefix operator, we want to semi-peek into the next character to get a line
            // in on whether we're EOF
            int_type _ch = stream->get();

            if (_ch == traits_type::eof())  // We don't expect to get here, since we tag
                stream = NULLPTR;
        }

        return *this;
    }

    class proxy
    {
        friend class istream_iterator;
    };
};

}}