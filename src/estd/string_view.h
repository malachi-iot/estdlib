#pragma once

#include "string.h"
#include "internal/fwd/string_view.h"

namespace estd {

namespace detail {

template <ESTD_CPP_CONCEPT(internal::StringPolicy) Policy>
class basic_string_view :
    public detail::basic_string<internal::impl::allocated_array<
            layer3::allocator<const typename Policy::char_traits::char_type, typename Policy::size_type>, Policy> >
{
    typedef detail::basic_string<internal::impl::allocated_array<
        layer3::allocator<const typename Policy::char_traits::char_type, typename Policy::size_type>, Policy> >
        base_type;

    typedef typename base_type::allocator_type allocator_type;

    // DEBT: InitParam works, but are probably better served by something like
    // in_place_t.  InitParam was done to get around lack of variadic forwarding
    typedef typename allocator_type::InitParam init_param_t;

public:
    typedef typename base_type::size_type size_type;
    typedef typename base_type::pointer pointer;
    typedef typename base_type::const_pointer const_pointer;

    // DEBT: Mainly to satisfy constraints observed by v0::impl::basic_sviewbuf.
    // There theoretically could be utility here in that a highly constrained
    // basic_string may generate a similarly constrained string_view (perhaps with
    // compile time sizing)
    using view_type = basic_string_view<Policy>;

    // As per spec, a no-constructor basic_string_view creates a null/null
    // scenario
    basic_string_view() : base_type(init_param_t(NULLPTR, 0)) {}

    ESTD_CPP_CONSTEXPR_RET basic_string_view(const_pointer s, size_type count) :
        base_type(init_param_t(s, count))
    {

    }

    // C-style null terminated string
    ESTD_CPP_CONSTEXPR_RET basic_string_view(const_pointer s) :
        base_type(init_param_t(s, strlen(s)))
    {

    }

    template <class Policy2>
    constexpr basic_string_view(
        const basic_string_view<Policy2>& other) :
        // NOTE: Similar to layer3::string init issue, other.size() may properly resulit
        // in narrowing warnings
        base_type(init_param_t(other.data(), other.size()))
    {

    }


    basic_string_view(const basic_string_view& other)
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        = default;
#else
        : base_type(other)
    {
    }
#endif

    /*
    template <class Policy2>
    basic_string_view& operator=(const basic_string_view<Policy2>& other)
    {
        // DEBT: Placement new far from ideal here.  An allocator re-init
        // is what we really need
        return * new (this) basic_string_view(other);
    }   */


#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
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

    // DEBT: Stop sprinkling all these 'data' methods around and instead inspect the
    // allocator to see if it's locking or not and disable or enable 'data' methods accordingly
    pointer data() { return base_type::lock(); }
    ESTD_CPP_CONSTEXPR_RET const_pointer data() const { return base_type::clock(); }

    // DEBT: Similar to above, for scenarios which are never gonna be locking (like layer strings)
    // we don't need the fancy locking iterator.  So continue to plumb the depths of locking_accessor,
    // base allocators and friends to smooth this out
    typedef pointer iterator;
    typedef const_pointer const_iterator;

    ESTD_CPP_CONSTEXPR_RET const_iterator begin() const { return data(); }
    ESTD_CPP_CONSTEXPR_RET const_iterator end() const { return data() + base_type::size(); }


#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    CONSTEXPR
#endif
    basic_string_view substr(
            size_type pos = 0,
            size_type count = base_type::npos) const
    {
        // DEBT: Use underlying data()/clock(), although it's noteworthy that
        // this seemingly clumsy approach *might* work for private/moveable basic_string_view
        // with some minor adjustments

        basic_string_view copy(*this);

        copy.remove_prefix(pos);
        if(count != base_type::npos)
        {
            allocator_type& a = copy.get_allocator();

            a.set_size_exp(true, count);
        }

        return copy;
    }
#endif
};

}

typedef basic_string_view<char> string_view;


}
