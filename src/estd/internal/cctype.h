#pragma once

namespace estd { namespace internal {

inline int ascii_isdigit(int ch)
{
    return '0' <= ch && ch <= '9';
}

inline int ascii_isspace(int ch)
{
    return ch == ' ';
}


}}