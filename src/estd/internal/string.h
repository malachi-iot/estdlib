#pragma once

#if __cpp_lib_string_view
#include <string_view>
#endif

#include "fwd/string.h"
#include "layer1/string.h"
#include "layer2/string.h"
#include "layer3/string.h"
#include "string/hash.h"
#include "string/operators.h"

#include "dynamic_array.h"

namespace estd { namespace internal {

#if __cpp_lib_string_view
// TODO: Consider upgrading this guy to work with estd things too, though may be superfluous
template <class SV, class Traits>
struct string_view_like
{
    using char_type = typename Traits::char_type;

    static constexpr bool value =
        is_convertible<
            const SV&,
            std::basic_string_view<char_type, Traits>>::value &
        is_convertible<
            const SV&,
            const char_type*>::value == false;
};
#endif

}}

namespace estd { namespace detail {

template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
class basic_string : public internal::dynamic_array<Impl>
{
protected:
    using base_type = internal::dynamic_array<Impl>;
    using typename base_type::helper;

public:
    using typename base_type::size_type;

    typedef typename base_type::allocator_type  allocator_type;
    typedef typename base_type::impl_type::policy_type policy_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename policy_type::char_traits traits_type;
    typedef typename allocator_type::pointer pointer;
    // DEBT: Get this from someone's traits
    //typedef typename allocator_type::const_pointer const_pointer;
    typedef const value_type* const_pointer;

    static constexpr size_type npos = (size_type) -1;

protected:
    ESTD_CPP_FORWARDING_CTOR(basic_string)

    template <typename InputIt>
    int compare(InputIt s, size_type s_size) const
    {
        size_type raw_size = base_type::size();

        if(raw_size < s_size) return -1;
        if(raw_size > s_size) return 1;

        // gets here if size matches
        return helper::compare(*this, s, s_size);
    }

    // +++ non-locking helpers, protected so only conforming child classes expose them

    ESTD_CPP_CONSTEXPR(17) pointer data()
    {
        return base_type::lock();
    }

    constexpr const_pointer data() const
    {
        return base_type::clock();
    }

    // A little clumsy since basic_string_view treats everything as const already,
    // so if we are converting from a const_string we have to remove const from CharT
    //using view_type = basic_string_view<policy_type>;
    using view_type = estd::basic_string_view<value_type, traits_type>;

    /*
    constexpr operator view_type() const
    {
        return view_type(data(), base_type::size());
    }   */

    // ---

public:
    basic_string() = default;

    constexpr size_type length() const { return base_type::size(); }

    template <class Impl2>
    int compare(const internal::allocated_array<Impl2>& a) const
    {
        // NOTE: Underlying compare never pays attention to null termination,
        // so we are safe comparing against a non-string here
        
        int result = compare(a.clock(), a.size());
        a.cunlock();

        return result;
    }

    // compare to a C-style string
    int compare(const_pointer s) const
    {
        return compare(s, strlen(s));
    }


    // compare to a C-style string
    bool starts_with(const_pointer compare_to) const
    {
        return helper::starts_with(*this, compare_to);
    }



    // Keeping this as I expect to eventually need a string/char traits aware
    // version of the character-by-character comparison
    template <class Impl2>
    bool starts_with(const internal::allocated_array<Impl2>& compare_to) const  // NOLINT
    {
        return base_type::starts_with(compare_to);
    }

    size_type find(value_type ch, size_type pos = 0) const
    {
        // DEBT: Use helper to optimize this for the particular kind of
        // string we're inspecting, or possibly use iterators if they'll
        // do things efficiently here

        const_pointer data = base_type::clock() + pos;
        const size_type size = base_type::size();

        while(pos < size)
        {
            if(*data++ == ch)
            {
                base_type::cunlock();
                return pos;
            }

            ++pos;
        }

        base_type::cunlock();

        return npos;
    }

    // NOT READY YET
    ///
    /// @param s incoming string to find
    /// @param pos last position in 'this' string to evaluate from
    /// @param count length of incoming 's'
    /// @return
    size_type find_last_of(const_pointer s, size_type pos, size_type count) const
    {
        if(pos == npos) pos = base_type::size();

        // if our length is less than requested string, we'll never match anyway
        // so abort
        if(pos < count) return npos;

        pos -= count;

        const_pointer data = base_type::clock() + pos;

        for(; pos != npos; --pos, --data)
        {
            if(memcmp(s, data, count) == 0) return pos;
        }

        base_type::cunlock();

        return npos;
    }

    size_type find_last_of(const_pointer s, size_type pos = npos) const
    {
        size_type count = traits_type::length(s);

        return find_last_of(s, pos, count);
    }

    template <class Impl2>
    size_type find_last_of(const basic_string<Impl2>& s, size_type pos = npos) const
    {
        const_pointer data = s.clock();
        size_type r = find_last_of(data, pos, s.size());
        s.cunlock();
        return r;
    }

    // --- writeable operations below
    // Be mindful that detail::basic_string is basic class of string_view too, so these are disabled in that
    // context

    basic_string& erase(size_type index = 0, size_type count = npos)
    {
        static_assert(base_type::policy_type::is_constant() == false, "This class is read only");

        size_type size_minus_index = base_type::size() - index;
        // NOTE: A bit tricky, if we don't use helper size_minus_index, template
        // resolution fails, presumably because the math operation implicitly
        // creates an int
        size_type to_remove_count = estd::min(count, size_minus_index);

        base_type::_erase(index, to_remove_count);

        return *this;
    }

    template <class Impl2>
    basic_string& operator=(const internal::allocated_array<Impl2>& copy_from)   // NOLINT
    {
        static_assert(base_type::policy_type::is_constant() == false, "This class is read only");

        base_type::assign(copy_from);
        return *this;
    }

    basic_string& operator=(const_pointer s)
    {
        static_assert(base_type::policy_type::is_constant() == false, "This class is read only");

        // DEBT: This can be optimized
        base_type::assign(s, strlen(s));
        return *this;
    }

    // DEBT: Does not conform to 'strong exception safety guarantee'
    template <class AppendResult>
    static void assert_append(AppendResult) //typename base_type::append_result r)
    {
        static_assert(base_type::policy_type::is_constant() == false, "This class is read only");

#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        if(r.has_value() == false)
        {
#if __cpp_exceptions
            throw std::length_error("Could not allocate enough memory");
#else
            std::abort();
#endif
        }
#endif
    }

    ESTD_CPP_CONSTEXPR(14) basic_string& operator+=(value_type c)
    {
        assert_append(base_type::push_back(c));
        return *this;
    }

    // DEBT: A little too permissive, though is OK since append itself is stricter
    template <class T>
    inline ESTD_CPP_CONSTEXPR(14) basic_string& operator+=(T rhs)
    {
        append(rhs);
        return *this;
    }


    template <class ForeignImpl>
    basic_string& operator=(const experimental::private_array<ForeignImpl>& copy_from)   // NOLINT
    {
        operator=(copy_from);
        return *this;
    }

    basic_string& append(size_type count, value_type c) // NOLINT
    {
        // NOTE: Minor optimization opportunity (don't push around size/null term a bunch of times)
        while(count--) *this += c;

        return *this;
    }

    template <class Impl2>
    basic_string& append(const internal::allocated_array<Impl2>& str)   // NOLINT
    {
        assert_append(base_type::append(str));
        return *this;
    }

    basic_string& append(const_pointer s, size_type count)  // NOLINT
    {
        assert_append(base_type::append(s, count));
        return *this;
    }

    // DEBT: Helper can optimize this guy
    constexpr basic_string& append(const_pointer s)
    {
        return append(s, strlen(s));
    }

#if __cpp_lib_string_view
    template <class SV,
        class Enabled = enable_if_t<internal::string_view_like<SV, traits_type>::value>>
    ESTD_CPP_CONSTEXPR(14) basic_string& append(const SV& s)
    {
        // A bit of a gap here, we don't gauruntee a std::basic_string_view here,
        // so the possibility exists that 's' doesn't have data() and size()
        append(s.data(), s.size());
        return *this;
    }
#endif
};

// DEBT: move this guy out to string/operators.h
template <ESTD_CPP_CONCEPT(concepts::v1::impl::String) Impl>
ESTD_CPP_CONSTEXPR_RET bool operator ==(
    const basic_string<Impl>& lhs,
    typename basic_string<Impl>::const_pointer rhs)
{
    return lhs.compare(rhs) == 0;
}

}}
