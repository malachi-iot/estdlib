#pragma once

#include "streambuf.h"
#include "../../ostream.h"

namespace estd {

typedef estd::internal::basic_ostream<estd::arduino_streambuf> arduino_ostream;

namespace internal {

template<class TStreambuf, class TBase>
inline basic_ostream<TStreambuf, TBase>& operator <<(
    basic_ostream<TStreambuf, TBase>& os, const __FlashStringHelper* rhs)
{
    int sz_rhs = strlen_P((const char*)rhs);

    os.write((const char*) rhs, sz_rhs);

    return os;

    // DEBT: Normal null-terminated flavor doesn't behave right, look into why
    //return operator <<(os, (const char*) rhs);
}


}


}


template<class TImpl> inline estd::arduino_ostream& PRINT_TEST(
    estd::arduino_ostream& os, const __FlashStringHelper* rhs)
{
    return operator <<(os, (const char*) rhs);
}


