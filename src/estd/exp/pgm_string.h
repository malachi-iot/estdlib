#include <avr/pgmspace.h>

#include <estd/internal/fwd/variant.h>
#include <estd/internal/dynamic_array.h>
#include "../string.h"

#include "../internal/macro/push.h"

namespace estd {

namespace internal {

namespace impl {

template <size_t N>
struct pgm_allocator2 :
    estd::internal::single_fixedbuf_allocator<
        char,
        N,
        const char*>
{
    using base_type = estd::internal::single_fixedbuf_allocator<
        char,
        N,
        const char*>;

    ESTD_CPP_FORWARDING_CTOR(pgm_allocator2)

    // DEBT: Kinda sloppy, exposing this this way, but it gets the job done
    // - Not deriving from pgn_allocator2 itself because then we get ambiguities
    // for things like const_pointer
    ESTD_CPP_CONSTEXPR_RET typename base_type::const_pointer data(
        typename base_type::size_type offset = 0) const
    {
        return base_type::data(offset);
    }
};

struct pgm_allocator
{
    using value_type = char;
    using pointer = const PROGMEM char*;
    using handle_type = pointer;
};


template <class T>
struct pgm_allocator_traits
{
    using value_type = T;
    using pointer = const PROGMEM value_type*;
    using const_pointer = pointer;
    using handle_type = pointer;
    using handle_with_offset = pointer;
    using size_type = uint16_t;

    static CONSTEXPR bool is_stateful_exp = false;
    static CONSTEXPR bool is_locking_exp = false;
};

enum class PgmPolicyType
{
    Buffer,
    String
};

// type=String, N = max = null terminated
// type=String, N != max = fixed compile time size
// type=Buffer, N = max = runtime sized
// type=Buffer, N != max = fixed compile time size
template <class T = char, PgmPolicyType type_ = PgmPolicyType::String,
    size_t N = estd::internal::variant_npos()>
struct PgmPolicy;

template <size_t N>
struct PgmPolicy<char, PgmPolicyType::String, N> : pgm_allocator_traits<char>
{
    using char_traits = estd::char_traits<char>;

    static constexpr size_t size() { return N; }

    // DEBT: Align this nomenclature with how 'span' does it,
    // since we're kind of making a fancy span here
    // DEBT: Along those lines, consider making a private_span alongside
    // private_array
    static constexpr bool null_terminated = N == estd::internal::variant_npos();
};

/*
 * TODO: Forgot to consider null term vs fixed size vs runtime size
 * Thinking runtime size just shouldn't be supported for pgm_string,
 * but low level private_array will likely want it eventually
template <>
struct PgmPolicy<estd::internal::variant_npos()> : pgm_allocator_traits
{
    using char_traits = estd::char_traits<char>;
    size_t size_;

    constexpr size_t size() const { return size_; }
    static constexpr bool has_extent = false;
};*/


}

}

namespace experimental {

template <class T, bool near = true>
class pgm_accessor;

template <typename T, bool near = true>
T pgm_read(const void* address);

template <>
char pgm_read<char>(const void* address)
{
    return pgm_read_byte_near(address);
}

template <>
uint16_t pgm_read<uint16_t>(const void* address)
{
    return pgm_read_word_near(address);
}

template <>
uint32_t pgm_read<uint32_t>(const void* address)
{
    return pgm_read_dword_near(address);
}

// Experimenting with strlen-less end() code, but it seems to gain nothing
// really
#define FEATURE_ESTD_PGM_EXP_IT 0

// Dogfooding in allocator.  So far it just seems to make it more complicated,
// but a feeling tells me this will be useful
#define FEATURE_ESTD_PGM_ALLOCATOR 1


template <class T>
class pgm_accessor<T> : protected internal::impl::pgm_allocator_traits<T>
{
    using base_type = internal::impl::pgm_allocator_traits<T>;
protected:
    using typename base_type::const_pointer;
    using typename base_type::value_type;

    const_pointer p;

    value_type value() const { return pgm_read<T>(p); }

#if FEATURE_ESTD_PGM_EXP_IT
    const bool is_null() const
    {
        return p == nullptr || value() == 0;
    }
#endif

public:
    constexpr pgm_accessor(const_pointer p) : p{p} {}

    value_type operator*() const { return value(); }

    //constexpr
    bool operator==(const pgm_accessor& compare_to) const
    {
#if FEATURE_ESTD_PGM_EXP_IT
        if(is_null() && compare_to.is_null()) return true;
#endif
        return p == compare_to.p;
    }

    //constexpr
    bool operator!=(const pgm_accessor& compare_to) const
    {
#if FEATURE_ESTD_PGM_EXP_IT
        if(!(is_null() && compare_to.is_null())) return true;
#endif
        return p != compare_to.p;
    }
};

template <class T, size_t N>
struct private_array_base : 
    estd::internal::impl::PgmPolicy<
        T, internal::impl::PgmPolicyType::String, N>
{
    using base_type = estd::internal::impl::PgmPolicy<T,
        internal::impl::PgmPolicyType::String, N>;

    using typename base_type::size_type;
    using typename base_type::const_pointer;
    using base_type::value_type;

#if FEATURE_ESTD_PGM_ALLOCATOR
    // data_ was working, but let's dogfood a bit
    internal::impl::pgm_allocator2<N> alloc;
    //using alloc_type = internal::impl::pgm_allocator2<N>;

    ESTD_CPP_CONSTEXPR_RET const_pointer data(size_type pos = 0) const
    {
        return alloc.data(pos);
    }

    constexpr private_array_base(const_pointer data) :
        alloc(data)
    {}
#else
    const_pointer data_;

    ESTD_CPP_CONSTEXPR_RET const_pointer data(size_type pos = 0) const
    {
        return data_ + pos;
    }

    constexpr private_array_base(const_pointer data) :
        data_{data}
    {}

#endif

    using accessor = pgm_accessor<char>;

    struct iterator : accessor
    {
        using typename accessor::const_pointer;

        constexpr iterator(const_pointer p) : accessor(p) {}

        iterator& operator++()
        {
            ++p;
            return *this;
        }

        accessor operator++(int)
        {
            accessor temp{p};
            ++p;
            return temp;
        }

        constexpr iterator operator+(int adder) const
        {
            return { p + adder };
        }

        iterator& operator+=(int adder)
        {
            p += adder;
            return *this;
        }
    };

    constexpr iterator begin() const { return { data() }; }

    accessor operator[](size_t index)
    {
        return accessor { data(index) };
    }
};


template<size_t N>
struct private_array<estd::internal::impl::PgmPolicy<char,
    internal::impl::PgmPolicyType::String, N>> :
    private_array_base<char, N>
{
    using base_type = private_array_base<char, N>;

    using typename base_type::size_type;
    using typename base_type::const_pointer;
    using typename base_type::value_type;
    using typename base_type::iterator;

    using const_iterator = iterator;

    constexpr private_array(const_pointer data) : base_type(data) {}

    size_type size() const
    {
        return base_type::null_terminated ?
            strnlen_P(base_type::data(), 256) :
            base_type::size();
    }

    constexpr iterator end() const
    {
        return { base_type::data(size()) };
    }

    // copies without bounds checking
    void copy_ll(char* dest, size_type count, size_type pos = 0) const
    {
        //iterator source(base_type::data_ + pos);
        iterator source(base_type::data(pos));

        estd::copy_n(source, count, dest);
    }

    size_type copy(char* dest, size_type count, size_type pos = 0) const
    {
        const size_type _end = estd::min(count, size());
        copy_ll(dest, _end, pos);

        //memcpy_P(dest, base_type::data_ + pos, count);
        
        return _end;
    }


    /* TODO
    template <class TImpl2>
    int compare(const internal::allocated_array<TImpl2>& s) const
    {
        return -1;
    }   */


    int compare(const char* s) const
    {
        if(base_type::null_terminated)
        {
            // FIX: Not fully checked to see if int result aligns with
            // dynamic_array flavor outside of 0/not 0
            return strcmp_P(s, base_type::data());
        }
        else
        {
            // DEBT: do more of a memcmp here
            return -1;
        }
    }

    private_array& operator=(const private_array& copy_from)
    {
        base_type::data() = copy_from.data();
        return *this;
    }
};

}

namespace internal {

template <>
struct basic_string<impl::pgm_allocator, impl::PgmPolicy<char>> :
    experimental::private_array<impl::PgmPolicy<char>>
{
    using base_type = experimental::private_array<impl::PgmPolicy<char>>;
    using allocator_type = impl::pgm_allocator;
    using allocator_traits = impl::pgm_allocator_traits<char>;
    using typename base_type::size_type;

    size_type length() const { return base_type::size(); }

    constexpr basic_string(const char* const s) : base_type(s) {}

#ifdef ARDUINO
    // UNTESTED
    constexpr basic_string(const __FlashStringHelper* s) :
        base_type(reinterpret_cast<const char*>(s))
    {}
#endif
};


}

template <>
struct allocator_traits<internal::impl::pgm_allocator> :
    internal::impl::pgm_allocator_traits<char>
{

};




//template <size_t N>
//using pgm_string = layer2::basic_string<char, N, false, char_traits<char>, internal::impl::PgmPolicy>;

/*
using pgm_string = basic_string<char, char_traits<char>,
    internal::impl::pgm_allocator,
    internal::impl::PgmPolicy>; */

struct pgm_string : basic_string<char, estd::char_traits<char>,
    internal::impl::pgm_allocator,
    internal::impl::PgmPolicy<char>>
{
    using base_type = basic_string<char, estd::char_traits<char>,
        internal::impl::pgm_allocator,
        internal::impl::PgmPolicy<char>>;

    constexpr pgm_string(const char* const s) : base_type(s) {}
};

}

#include "../internal/macro/pop.h"
