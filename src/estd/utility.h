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

    typedef T1 first_type;
    typedef T2 second_type;

    //pair(T1& first, T2& second) : first(first), second(second) {}
};

}
