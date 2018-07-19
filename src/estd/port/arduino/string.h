#pragma once

//namespace estd {

template< class CharT, class Traits, class Alloc, class Policy >
    estd::basic_string<CharT,Traits,Alloc,Policy>&
        operator+=(estd::basic_string<CharT,Traits,Alloc,Policy>& lhs,
                   const __FlashStringHelper* rhs )
{
    lhs += "";
    return lhs;
}

//}
