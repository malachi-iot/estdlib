#pragma once

#include "streambuf.h"
#include "../../internal/ostream.h"

namespace estd {

typedef estd::detail::basic_ostream<estd::arduino_ostreambuf> arduino_ostream;

namespace internal {

template<class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator <<(
    basic_ostream<TStreambuf, TBase>& os, const __FlashStringHelper* rhs)
{
    os.rdbuf()->underlying().print(rhs);

    return os;

    // DEBT: Normal null-terminated flavor doesn't behave right, look into why
    //return operator <<(os, (const char*) rhs);
}


}


}


