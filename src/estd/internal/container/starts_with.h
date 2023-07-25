#pragma once

namespace estd { namespace internal {

// Compare two containers, returning:
// - true if rhs is completely found
// - false if rhs is not found or lhs ends before determination is made
template <typename It1, typename It2>
bool starts_with_n(It1 lhs, It2 rhs, int count_lhs, int count_rhs)
{
    // DEBT: Using signed int here is fiddly, since incoming count likely won't be signed

    while(count_lhs-- && count_rhs--)
        if(*lhs++ != *rhs++)
        {
            return false;
        }

    // if compare_to is longer than we are, then it's also a fail
    return count_lhs != -1;
}


// Compare two strings, assumes rhs is null terminated
template <typename It1, typename It2>
bool starts_with_n(It1 lhs, It2 rhs, int count)
{
    // DEBT: Using signed int here is fiddly, since incoming count likely won't be signed

    while(*rhs != 0 && count--)
        if(*lhs++ != *rhs++)
        {
            return false;
        }

    // if compare_to is longer than we are, then it's also a fail
    return count != -1;
}

// Both are null-terminated here
// DEBT: Make bounded version
template <typename It1, typename It2>
bool starts_with(It1 lhs, It2 rhs)
{
    for(;;)
    {
        // Reaching the end of compare_to string = succeeded
        if(*rhs == 0)   return true;

            // Reaching the end of original string after not reaching compare_to = failed
            // through entertaining magic, this line is automatically handled below since
            // we know rhs now cannot be 0, meaning that if lhs IS zero it will never match rhs,
            // thus returning false
            //else if(*lhs == 0)   return false;

            // While still searching, if we get mismatched characters, we're done
        else if(*lhs++ != *rhs++)    return false;
    }
}

}}