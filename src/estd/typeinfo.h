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

namespace experimental {

template <unsigned N, typename Enabled>
inline const char* type_name_helper(unsigned idx);

struct type_eof_tag {};

template <unsigned N>
struct reverse_type_info : type_eof_tag
{
    static const char* name() { return nullptr; }

    //static const char* test1(unsigned index) { return nullptr; }
};

template <>
struct reverse_type_info<2>
{
    static const char* name() { return "index2"; }
};

template <unsigned N, estd::enable_if_t<
    estd::is_base_of<type_eof_tag, reverse_type_info<N> >::value, int
    > = 0>
inline const char* type_name_helper(unsigned idx)
{
    return nullptr;
}

template <unsigned N, estd::enable_if_t<
    //true, int
    !(estd::is_base_of<type_eof_tag, reverse_type_info<N> >::value), int
    > = 0>
inline const char* type_name_helper(unsigned idx)
{
    switch(idx)
    {
        case N:
            return reverse_type_info<N>::name();

        case N + 1:
            return reverse_type_info<N + 1>::name();

        case N + 2:
            return reverse_type_info<N + 2>::name();

        case N + 3:
            return reverse_type_info<N + 3>::name();

        default:
            return type_name_helper<N + 4>(idx);
    }
}

}

}