#pragma once

namespace estd {

#define ESTD_Q(x) #x
#define ESTD_QUOTE(x) Q(x)

#define ESTD_TYPEINFO_HELPER(type)  \
template <>                         \
struct type_info<type>              \
{                                   \
    static const char* name()       \
    { return ESTD_Q(type); }        \
                                    \
    CONSTEXPR                       \
    static std::size_t hashcode()   \
    { return __COUNTER__; }         \
};

template <class T>
struct type_info;

template <>
struct type_info<int>
{
    static const char* name() { return "int"; }

    static std::size_t hashcode() { return __COUNTER__; }
};

ESTD_TYPEINFO_HELPER(long)
ESTD_TYPEINFO_HELPER(bool)
ESTD_TYPEINFO_HELPER(int8_t)
//ESTD_TYPEINFO_HELPER(uint8_t)
ESTD_TYPEINFO_HELPER(char)
ESTD_TYPEINFO_HELPER(unsigned char)

}