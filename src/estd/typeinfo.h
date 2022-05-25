#pragma once

namespace estd {

namespace experimental {

template <unsigned group, unsigned idx>
constexpr const char* reverse_data_type_name();

struct type_eof_tag {};

template <unsigned group, unsigned idx>
struct reverse_type_info : type_eof_tag
{
    static const char* name() { return nullptr; }
};


ESTD_CPP_CONSTEXPR_RET size_t encode_type_index(unsigned group, unsigned idx)
{
    return group << 8 | idx;
}

typedef estd::pair<unsigned, unsigned> decoded_type_index_type;

ESTD_CPP_CONSTEXPR_RET estd::pair<unsigned, unsigned> decode_type_index(unsigned idx)
{
    return estd::pair<unsigned, unsigned>{idx >> 8, idx & 0xFF};
}

struct type_info_groups
{
    enum values
    {
        system = 0,
        library = 10,
        application = 20,
        user = 30
    };
};


}

#define ESTD_Q(x) #x
#define ESTD_QUOTE(x) Q(x)

#define ESTD_TYPEINFO_HELPER(type, group, idx)  \
template <>                         \
struct type_info<type>              \
{                                   \
    static const char* name()       \
    { return ESTD_Q(type); }        \
                                    \
    ESTD_CPP_CONSTEXPR_RET                       \
    static std::size_t hashcode()   \
    { return experimental::encode_type_index(group, idx); }    \
};  \
                                                \
namespace experimental {                            \
template <> struct reverse_type_info<group, idx>    \
{   \
    static const char* name() { return type_info<type>::name(); }   \
};  \
}

template <class T>
struct type_info;

template <>
struct type_info<int>
{
    static const char* name() { return "int"; }

    static std::size_t hashcode() { return __COUNTER__; }
};

ESTD_TYPEINFO_HELPER(long, 0, 0)
ESTD_TYPEINFO_HELPER(bool, 0, 1)
ESTD_TYPEINFO_HELPER(int8_t, 0, 2)
//ESTD_TYPEINFO_HELPER(uint8_t)
ESTD_TYPEINFO_HELPER(char, 0, 3)
ESTD_TYPEINFO_HELPER(unsigned char, 0, 4)

namespace experimental {

template <unsigned N, typename Enabled>
inline const char* type_name_helper(unsigned idx);

template <unsigned group, typename Enabled>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx);


template <unsigned group, unsigned N, estd::enable_if_t<
    estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value, int
    > = 0>
inline const char* type_name_helper(unsigned idx)
{
    return nullptr;
}

template <unsigned group, unsigned N, estd::enable_if_t<
    //true, int
    !(estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value), int
    > = 0>
inline const char* type_name_helper(unsigned idx)
{
    switch(idx)
    {
        case N:
            return reverse_type_info<group, N>::name();

        case N + 1:
            return reverse_type_info<group, N + 1>::name();

        case N + 2:
            return reverse_type_info<group, N + 2>::name();

        case N + 3:
            return reverse_type_info<group, N + 3>::name();

        default:
            return type_name_helper<group, N + 4>(idx);
    }
}

template <unsigned group, estd::enable_if_t<
    estd::is_base_of<type_eof_tag, reverse_type_info<group, 0> >::value, int
> = 0>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx)
{
    return nullptr;
}

template <unsigned group, estd::enable_if_t<
    //true, int
    !(estd::is_base_of<type_eof_tag, reverse_type_info<group, 0> >::value), int
> = 0>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx)
{
    switch(grp_idx)
    {
        case group:
            return type_name_helper<group, 0>(idx);

        case group + 1:
            return type_name_helper<group + 1, 0>(idx);

        case group + 2:
            return type_name_helper<group + 2, 0>(idx);

        case group + 3:
            return type_name_helper<group + 3, 0>(idx);

        default:
            return type_name_helper2<group + 4>(grp_idx, idx);
    }
}

inline const char* type_name_helper3(unsigned _idx)
{
    decoded_type_index_type d = decode_type_index(_idx);
    return type_name_helper2<0>(d.first, d.second);
}

}

}