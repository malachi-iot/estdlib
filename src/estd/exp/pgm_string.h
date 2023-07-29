#pragma once

#include <avr/pgmspace.h>

#include "pgm/allocator.h"
#include "pgm/read.h"

#include <estd/internal/fwd/variant.h>
#include <estd/internal/dynamic_array.h>
#include <estd/internal/container/locking_accessor.h>
#include <estd/internal/container/iterator.h>
#include "../string.h"
#include <estd/ostream.h>

#include "../internal/macro/push.h"

namespace estd {

namespace internal {

namespace impl {

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


template <size_t N>
using PgmStringPolicy = impl::PgmPolicy<char, impl::PgmPolicyType::String, N>;


}

namespace experimental {

template <class T, bool near = true>
class pgm_accessor;

// "legacy" flavor, phasing out
template <class T>
class pgm_accessor<T> : protected internal::impl::pgm_allocator_traits<T>
{
    using base_type = internal::impl::pgm_allocator_traits<T>;
protected:
    using typename base_type::const_pointer;
    using typename base_type::value_type;

    const_pointer p;

    value_type value() const { return internal::pgm_read<T>(p); }

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
    using allocator_traits = internal::impl::pgm_allocator_traits<T>;

    // DEBT: Dummy value so that regular estd::basic_string gets its
    // dependency satisfied
    using append_result = bool;

#if FEATURE_ESTD_PGM_ALLOCATOR
    typedef internal::impl::pgm_allocator2<T, N> allocator_type;
    // data_ was working, but let's dogfood a bit
    allocator_type alloc;

    ESTD_CPP_CONSTEXPR_RET const_pointer data(size_type pos = 0) const
    {
        return alloc.data(pos);
    }

    constexpr private_array_base(const_pointer data) :
        alloc(data)
    {}

    allocator_type& get_allocator() { return alloc; }
    const_pointer offset(unsigned pos) const 
    {
        return alloc.data(pos);
    }
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

#if FEATURE_ESTD_PGM_ALLOCATOR
    using accessor = internal::impl::pgm_accessor2<T>;
    using iterator = estd::internal::locking_iterator<
        allocator_type,
        accessor,
        estd::internal::locking_iterator_modes::ro >;
#else
    using accessor = pgm_accessor<T>;

    struct iterator : accessor
    {
        using typename accessor::const_pointer;

        constexpr iterator(const_pointer p) : accessor(p) {}

        iterator& operator++()
        {
            ++accessor::p;
            return *this;
        }

        accessor operator++(int)
        {
            accessor temp{accessor::p};
            ++accessor::p;
            return temp;
        }

        constexpr iterator operator+(int adder) const
        {
            return { accessor::p + adder };
        }

        iterator& operator+=(int adder)
        {
            accessor::p += adder;
            return *this;
        }
    };

    constexpr iterator create_iterator(size_t pos) const
    {
        return iterator(accessor(data(pos)));
    }

    constexpr iterator begin() const
    {
        return create_iterator(0);
    }

    accessor operator[](size_t index)
    {
        return accessor { data(index) };
    }
#endif
};


// Null terminated PGM space string core
template <class T, size_t N>
struct pgm_array_string : private_array_base<T, N>
{
    using base_type = private_array_base<T, N>;

    using typename base_type::size_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::value_type;
    using typename base_type::accessor;
    using typename base_type::iterator;
    using policy_type = internal::PgmStringPolicy<N>;

    using const_iterator = iterator;

    constexpr pgm_array_string(const_pointer data) : base_type(data) {}

    size_type size() const
    {
        return base_type::null_terminated ?
            strnlen_P(base_type::data(), 256) :
            base_type::size();
    }

    constexpr iterator end() const
    {
        return base_type::create_iterator(size());
    }

    // copies without bounds checking
    void copy_ll(value_type* dest, size_type count, size_type pos = 0) const
    {
#if FEATURE_ESTD_PGM_ALLOCATOR
        iterator source = base_type::create_iterator(pos);
#else
        //iterator source(base_type::data_ + pos);
        iterator source(base_type::data(pos));
#endif

        estd::copy_n(source, count, dest);
    }

    size_type copy(value_type* dest, size_type count, size_type pos = 0) const
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

    // Expects regular (non pgm) space string here
    // both compare_to and pgm_string must be null terminated
    bool starts_with(const char* compare_to) const
    {
        //internal::starts_with_n(begin(), compare_to, size());
        return internal::starts_with(base_type::begin(), compare_to);
    }

    // Effectively a shallow copy, since pgm_string largely represents a pointer
    // wrapper
    pgm_array_string& operator=(const pgm_array_string& copy_from)
    {
        base_type::data() = copy_from.data();
        return *this;
    }
};

template<size_t N>
struct private_array<estd::internal::impl::PgmPolicy<char,
    internal::impl::PgmPolicyType::String, N>> :
    pgm_array_string<char, N>
{
    using base_type = pgm_array_string<char, N>;

    constexpr private_array(typename base_type::const_pointer p) :
        base_type(p) {}
};


}

namespace internal {

template <size_t N>
struct basic_string<impl::pgm_allocator, PgmStringPolicy<N>> :
    experimental::private_array<PgmStringPolicy<N>>
{
    using base_type = experimental::private_array<PgmStringPolicy<N>>;
    using allocator_type = impl::pgm_allocator;
    using allocator_traits = impl::pgm_allocator_traits<char>;
    using typename base_type::size_type;

    size_type length() const { return base_type::size(); }

    // Expects PGM space string here
    constexpr basic_string(const char* const s) : base_type(s) {}

#ifdef ARDUINO
    // UNTESTED
    constexpr basic_string(const __FlashStringHelper* s) :
        base_type(reinterpret_cast<const char*>(s))
    {}
#endif
};

/*
template <size_t N>
struct basic_string2<experimental::pgm_array_string<char, N> >
{

};  */



}   // estd::internal

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

template <size_t N = internal::variant_npos()>
struct basic_pgm_string : basic_string<char, estd::char_traits<char>,
    internal::impl::pgm_allocator,
    internal::PgmStringPolicy<N>>
{
    using base_type = basic_string<char, estd::char_traits<char>,
        internal::impl::pgm_allocator,
        internal::PgmStringPolicy<N>>;

    constexpr basic_pgm_string(const char* const s) : base_type(s) {}
};


using pgm_string = basic_pgm_string<>;

// Special case insertion operator for arduino streams who can handle
// PGM space directly
template <size_t N>
constexpr arduino_ostream& operator <<(arduino_ostream& out,
    const experimental::pgm_array_string<char, N>& s)
{
    return out << reinterpret_cast<const __FlashStringHelper*>(s.data());
}

}

