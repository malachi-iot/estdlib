#include <avr/pgmspace.h>

#include <estd/internal/fwd/variant.h>
#include <estd/internal/dynamic_array.h>
#include "../string.h"

namespace estd {

namespace internal {

namespace impl {

struct pgm_allocator
{
    using value_type = char;
    using pointer = const PROGMEM char*;
    using handle_type = pointer;
};


struct pgm_allocator_traits
{
    using value_type = char;
    using pointer = const PROGMEM char*;
    using const_pointer = pointer;
    using handle_type = pointer;
    using handle_with_offset = pointer;
    using size_type = size_t;

    static CONSTEXPR bool is_stateful_exp = false;
    static CONSTEXPR bool is_locking_exp = false;
};

template <size_t N = estd::internal::variant_npos()>
struct PgmPolicy : pgm_allocator_traits
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

template<>
struct private_array<estd::internal::impl::PgmPolicy<>> :
    estd::internal::impl::PgmPolicy<>
{
    using base_type = estd::internal::impl::PgmPolicy<>;

    using base_type::size_type;
    using base_type::const_pointer;
    using base_type::value_type;

    const_pointer data_;

    size_type size() const
    {
        return base_type::null_terminated ?
            strnlen_P(data_, 256) :
            base_type::size();
    }

    class accessor
    {
    protected:
        pointer p;

    public:
        value_type operator*() const
        {
            return pgm_read_byte_near(p);
        }

        constexpr accessor(const_pointer p) : p{p} {}

        constexpr bool operator==(const accessor& compare_to) const
        {
            return p == compare_to.p;
        }

        constexpr bool operator!=(const accessor& compare_to) const
        {
            return p != compare_to.p;
        }
    };

    struct iterator : accessor
    {
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
    };

    using const_iterator = iterator;

    iterator begin() const { return { data_ }; }
    iterator end() const { return { data_ + size() }; }

    accessor operator[](size_t index)
    {
        return accessor { data_ + index };
    }

    size_type copy(char* dest, size_type count, size_type pos = 0)
    {
        // FIX: Doesn't pay attention to size()

        memcpy_P(dest, data_ + pos, count);
        return count;
    }
};

}

namespace internal {

template <>
struct basic_string<impl::pgm_allocator, impl::PgmPolicy<>> :
    experimental::private_array<impl::PgmPolicy<>>
{
    using allocator_type = impl::pgm_allocator;
    using allocator_traits = impl::pgm_allocator_traits;

    basic_string(const char* const s) {}
};


}

template <>
struct allocator_traits<internal::impl::pgm_allocator> :
    internal::impl::pgm_allocator_traits
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
    internal::impl::PgmPolicy<>>
{
    using base_type = basic_string<char, estd::char_traits<char>,
        internal::impl::pgm_allocator,
        internal::impl::PgmPolicy<>>;

    pgm_string(const char* const s) : base_type(s) {}
};

}

