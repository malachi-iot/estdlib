#pragma once

#include "streambuf.h"
#include "../../internal/ostream.h"

namespace estd {

using arduino_ostream = estd::detail::basic_ostream<estd::arduino_ostreambuf>;

namespace detail {

template<class Streambuf, class Base>
inline basic_ostream<Streambuf, Base>& operator <<(
    basic_ostream<Streambuf, Base>& os, const __FlashStringHelper* rhs)
{
    os.rdbuf()->underlying().print(rhs);

    return os;

    // DEBT: Normal null-terminated flavor doesn't behave right, look into why
    //return operator <<(os, (const char*) rhs);
}


}


}


