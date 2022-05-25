#pragma once

namespace estd {

#define ESTD_Q(x) #x
#define ESTD_QUOTE(x) Q(x)

#define ESTD_TYPEINFO_HELPER(type, idx)  \
template <>                         \
struct type_info<type>              \
{                                   \
    static const char* name()       \
    { return ESTD_Q(type); }        \
                                    \
    CONSTEXPR                       \
    static std::size_t hashcode()   \
    { return idx; }                 \
};

template <class T>
struct type_info;

template <>
struct type_info<int>
{
    static const char* name() { return "int"; }

    static std::size_t hashcode() { return __COUNTER__; }
};

ESTD_TYPEINFO_HELPER(long, 0)
ESTD_TYPEINFO_HELPER(bool, 1)
ESTD_TYPEINFO_HELPER(int8_t, 2)
//ESTD_TYPEINFO_HELPER(uint8_t)
ESTD_TYPEINFO_HELPER(char, 3)
ESTD_TYPEINFO_HELPER(unsigned char, 4)

namespace internal {

template <unsigned N>
struct reverse_type_info
{
    static const char* test1(unsigned index) { return nullptr; }
};

template <>
struct reverse_type_info<0>
{
    static const char* test1(unsigned index)
    {
        if(index == 0) return "test1";

        reverse_type_info<1>::test1(index);
    }
};


template <unsigned N>
inline const char* type_name_helper(unsigned idx)
{
    switch(idx)
    {
        case N:
            break;

        case N + 1:
            return type_name_helper<N>(idx);
    }
}

}

}