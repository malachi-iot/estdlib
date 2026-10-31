#pragma once

#include "internal/macros.h"
#include "internal/string.h"
#include "internal/fwd/string_view.h"
#include "policy/string.h"
#include "span.h"

#if FEATURE_STD_STRING_VIEW
#include <string_view>
#endif

namespace estd { namespace detail {

// Not aliasing this guy to std::string_view since shared base class with basic_string is a nice feature

template <ESTD_CPP_CONCEPT(internal::StringPolicy) Policy>
class basic_string_view :
    public detail::basic_string<internal::impl::allocated_array<
        layer3::allocator<const typename Policy::char_traits::char_type, typename Policy::size_type>, Policy>>
{
    using traits_type = typename Policy::char_traits;
    using base_type = detail::basic_string<internal::impl::allocated_array<
        layer3::allocator<const typename traits_type::char_type, typename Policy::size_type>, Policy>>;

    using typename base_type::allocator_type;

public:
    using base_type::data;
    using base_type::npos;
    using typename base_type::size_type;

    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;

    // DEBT: Mainly to satisfy constraints observed by v0::impl::basic_sviewbuf.
    // There theoretically could be utility here in that a highly constrained
    // basic_string may generate a similarly constrained string_view (perhaps with
    // compile time sizing)
    using view_type = basic_string_view;
    using span_type = span<value_type>;

    // As per spec, a no-constructor basic_string_view creates a null/null
    // scenario
    constexpr basic_string_view() : base_type(span_type{}) {}

    constexpr basic_string_view(const_pointer s, size_type count) :
        base_type(span_type(s, count))
    {

    }

    constexpr basic_string_view(const_pointer begin, const_pointer end) :
        base_type(span_type(begin, end))
    {

    }

    // C-style null terminated string
    constexpr basic_string_view(const_pointer s) :  // NOLINT
        // char_traits::length only constexpr with c++17 on
#if __cplusplus >= 201703L
        basic_string_view(s, traits_type::length(s))
#else
        basic_string_view(s, estd::strlen(s))
#endif
    {

    }

    template <class Policy2>
    constexpr basic_string_view(                    // NOLINT
        const basic_string_view<Policy2>& other) :
        // NOTE: Similar to layer3::string init issue, other.size() may properly result
        // in narrowing warnings
        base_type(span_type(other.data(), other.size()))
    {

    }

#if __cpp_lib_string_view
    // UNTESTED
    constexpr basic_string_view(const std::basic_string_view<value_type>& copy_from) :
        basic_string_view(copy_from.begin(), copy_from.end())
    {}

    // UNTESTED
    constexpr operator std::basic_string_view<value_type>() const
    {
        return { begin(), end() };
    }
#endif

    //basic_string_view(const basic_string_view& other) = default;

    /*
    template <class Policy2>
    basic_string_view& operator=(const basic_string_view<Policy2>& other)
    {
        // DEBT: Placement new far from ideal here.  An allocator re-init
        // is what we really need
        return * new (this) basic_string_view(other);
    }   */


    void remove_suffix(size_type n)
    {
        // FIX: Not right - reallocate does nothing in this context
        // basic_string_view length/size hangs off max_capacity
        // of fixed allocator
        //base_t::helper.reallocate(base_t::capacity() - n);
        //base_t::helper.size(base_t::helper.size() - n);
        allocator_type& a = base_type::get_allocator();

        a.set_size_exp(true, a.max_size() - n);
    }


    void remove_prefix(size_type n)
    {
        allocator_type& a = base_type::get_allocator();

        a.adjust_offset_exp(true, n);
    }

    // DEBT: Similar to above, for scenarios which are never gonna be locking (like layer strings)
    // we don't need the fancy locking iterator.  So continue to plumb the depths of locking_accessor,
    // base allocators and friends to smooth this out
    using iterator = pointer;
    using const_iterator = const_pointer;

    constexpr const_iterator begin() const { return data(); }
    constexpr const_iterator end() const { return data() + base_type::size(); }

    ESTD_CPP_CONSTEXPR(14) basic_string_view substr(
        size_type pos = 0,
        size_type count = npos) const
    {
        // DEBT: Use underlying data()/clock(), although it's noteworthy that
        // this seemingly clumsy approach *might* work for private/moveable basic_string_view
        // with some minor adjustments

        basic_string_view copy(*this);

        copy.remove_prefix(pos);
        if(count != npos)
        {
            allocator_type& a = copy.get_allocator();

            a.set_size_exp(true, count);
        }

        return copy;
    }

    ESTD_CPP_CONSTEXPR(14) basic_string_view subview(
        size_type pos = 0,
        size_type count = npos) const
    { return substr(pos, count); }
};

}}

namespace estd {

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;

}
