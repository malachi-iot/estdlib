#pragma once

namespace estd {

struct module_info
{
    const char* name;
};

namespace layer0 {

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

struct system_type_info_index
{
    enum values
    {
        i_int,
        i_long,
        i_bool,

        i_int8,
        i_uint8,
        i_int16,
        i_uint16,
        i_int32,
        i_uint32,
        i_char,
        i_uchar,

        end
    };
};


}

#define ESTD_Q(x) #x
#define ESTD_QUOTE(x) Q(x)

// DEBT: May need an alternate version if platform doesn't support
// static CONSTEXPR char* name_ here.

struct type_info_tag {};

#define ESTD_TYPEINFO_HELPER(type_name, group, idx)  \
template <>                         \
struct type_info<type_name> : type_info_tag  \
{                                               \
    static CONSTEXPR char* name_ = ESTD_Q(type_name); \
    static const char* name()       \
    { return name_; }        \
                                    \
    ESTD_CPP_CONSTEXPR_RET                       \
    static std::size_t hash_code()   \
    { return experimental::encode_type_index(group, idx); }    \
};  \
                                                \
namespace experimental {                            \
template <> struct reverse_type_info<group, idx>    \
{                                               \
    typedef type_info<type_name> type;       \
    static const char* name() { return type::name(); }   \
};  \
}

template <class T, module_info* module = nullptr>
struct type_info;

template <>
struct type_info<int>
{
    static const char* name() { return "int"; }

    static std::size_t hash_code() { return experimental::system_type_info_index::i_int; }
};

namespace experimental {
template <> struct reverse_type_info<0, experimental::system_type_info_index::i_int>
{
    static const char* name() { return type_info<int>::name(); }
};
}

ESTD_TYPEINFO_HELPER(long, 0, experimental::system_type_info_index::i_long)
ESTD_TYPEINFO_HELPER(bool, 0, experimental::system_type_info_index::i_bool)
ESTD_TYPEINFO_HELPER(int8_t, 0, experimental::system_type_info_index::i_int8)
//ESTD_TYPEINFO_HELPER(uint8_t)
ESTD_TYPEINFO_HELPER(char, 0, experimental::system_type_info_index::i_char)
ESTD_TYPEINFO_HELPER(unsigned char, 0, experimental::system_type_info_index::i_uchar)
ESTD_TYPEINFO_HELPER(int16_t, 0, experimental::system_type_info_index::i_int16)
ESTD_TYPEINFO_HELPER(uint16_t, 0, experimental::system_type_info_index::i_uint16)
// Collision with int on x64 debian gcc
//ESTD_TYPEINFO_HELPER(int32_t, 0, experimental::system_type_info_index::i_int32)
ESTD_TYPEINFO_HELPER(uint32_t, 0, experimental::system_type_info_index::i_uint32)

namespace experimental {

template <unsigned N, typename Enabled>
inline const char* type_name_helper(unsigned idx);

template <unsigned group, typename Enabled>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx);

/// More or less, "is there anybody out there" for the specified N-range in 'group'
/// \tparam group
/// \tparam N starting point
/// \tparam max end point, defaults to N + 16
// DEBT: Recursion tidily works here, but
// https://quuxplusone.github.io/blog/2018/07/23/metafilter/
// tells us iterating would be better
template <unsigned group, unsigned N, unsigned max = N + 16>
struct _reverse_type_in_range
{
    typedef typename estd::conditional<
        (N < max),
        _reverse_type_in_range<group, N + 1, max>,
        false_type
        >::type type;

    CONSTEXPR static bool value =
        type::value | !estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value;
};

#if UNUSED
/// If any type infos up to N + 3 are available, value is true
/// \tparam group
/// \tparam N
template <unsigned group, unsigned N>
struct reverse_type_in_range
{
    typedef typename estd::conditional<
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 1> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 2> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 3> >::value,
        true_type,
        false_type>::type type;

    static CONSTEXPR bool value =
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 1> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 2> >::value |
        !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 3> >::value;
};

template <unsigned group, unsigned N>
struct reverse_type_in_range2 : estd::conditional<
    !estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value |
    !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 1> >::value |
    !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 2> >::value |
    !estd::is_base_of<type_eof_tag, reverse_type_info<group, N + 3> >::value,
    true_type,
    false_type>
{
};
#endif





template <unsigned group, unsigned N, estd::enable_if_t<
    //estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value, int
    !_reverse_type_in_range<group, N>::value, int
    > = 0>
inline const char* type_name_helper(unsigned idx)
{
    return nullptr;
}

// Effectively a code generator for type name retrieval
template <unsigned group, unsigned N, estd::enable_if_t<
    //true, int
    // !(estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value), int
    _reverse_type_in_range<group, N>::value, int
    > = 0>
const char* type_name_helper(unsigned idx)
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

        case N + 4:
            return reverse_type_info<group, N + 4>::name();

        case N + 5:
            return reverse_type_info<group, N + 5>::name();

        case N + 6:
            return reverse_type_info<group, N + 6>::name();

        case N + 7:
            return reverse_type_info<group, N + 7>::name();

        default:
            return type_name_helper<group, N + 8>(idx);
    }
}

template <unsigned group, estd::enable_if_t<
    //estd::is_base_of<type_eof_tag, reverse_type_info<group, 0> >::value, int
    !_reverse_type_in_range<group, 0>::value, int
> = 0>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx)
{
    return nullptr;
}

template <unsigned group, estd::enable_if_t<
    // !(estd::is_base_of<type_eof_tag, reverse_type_info<group, 0> >::value), int
    _reverse_type_in_range<group, 0>::value, int
> = 0>
inline const char* type_name_helper2(unsigned grp_idx, unsigned idx)
{
    // DEBT: Hardcoded groups at 10 spaces apart, mapping to
    // experimental::system_type_info_index.  General idea is good but needs work
    switch(grp_idx)
    {
        case group:
            return type_name_helper<group, 0>(idx);

        case group + 10:
            return type_name_helper<group + 10, 0>(idx);

        case group + 20:
            return type_name_helper<group + 20, 0>(idx);

        case group + 30:
            return type_name_helper<group + 30, 0>(idx);

        default:
            return type_name_helper2<group + 40>(grp_idx, idx);
    }
}

inline const char* type_name_helper3(unsigned _idx)
{
    decoded_type_index_type d = decode_type_index(_idx);
    return type_name_helper2<0>(d.first, d.second);
}


template <unsigned group, unsigned N, typename F, estd::enable_if_t<
    //estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value, int
    !_reverse_type_in_range<group, N>::value, int
> = 0>
inline const char* type_dispatcher(unsigned idx, F&& f)
{
    return nullptr;
}

// Effectively a code generator for type name retrieval
template <unsigned group, unsigned N, typename F, estd::enable_if_t<
    //true, int
    // !(estd::is_base_of<type_eof_tag, reverse_type_info<group, N> >::value), int
    _reverse_type_in_range<group, N>::value, int
> = 0>
const char* type_dispatcher(unsigned idx, F&& f)
{
    switch(idx)
    {
        case N:
            return f(typename reverse_type_info<group, N>::type{});

        case N + 1:
            return f(typename reverse_type_info<group, N + 1>::type{});

        case N + 2:
            return f(typename reverse_type_info<group, N + 2>::type{});

        case N + 3:
            return f(typename reverse_type_info<group, N + 3>::type{});

        case N + 4:
            return f(typename reverse_type_info<group, N + 4>::type{});

        case N + 5:
            return f(typename reverse_type_info<group, N + 5>::type{});

        case N + 6:
            return f(typename reverse_type_info<group, N + 6>::type{});

        case N + 7:
            return f(typename reverse_type_info<group, N + 7>::type{});

        default:
            return type_dispatcher<group, N + 8>(idx, std::move(f));
    }
}

}

}   //layer0

}