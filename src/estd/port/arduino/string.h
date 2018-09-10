#pragma once

#include <Arduino.h>

//namespace estd {

template< class CharT, class Traits, class Alloc, class Policy >
    estd::basic_string<CharT,Traits,Alloc,Policy>&
        operator+=(estd::basic_string<CharT,Traits,Alloc,Policy>& lhs,
                   const __FlashStringHelper* rhs )
{
    int sz = lhs.size();
    int sz_rhs = strlen_P((const char*)rhs);
    lhs.resize(sz + sz_rhs);
    memcpy_P(lhs.lock(sz), rhs, sz_rhs);
    lhs.unlock();
    return lhs;
}

template< class CharT, class Traits, class Alloc, class Policy >
    estd::basic_string<CharT,Traits,Alloc,Policy>&
operator+=(estd::basic_string<CharT,Traits,Alloc,Policy>& lhs,
           const String& rhs )
{
    rhs.getBytes(lhs.lock(rhs.length()), lhs.size());
    lhs.unlock();
    return lhs;
}


//}
