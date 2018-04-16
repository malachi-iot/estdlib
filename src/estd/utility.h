#pragma once

// TODO: Utilize stock-standard std version of this if it's available

namespace estd {

template<
    class T1,
    class T2
> struct pair
{
    T1 first;
    T2 second;

    //pair(T1& first, T2& second) : first(first), second(second) {}
};

}
