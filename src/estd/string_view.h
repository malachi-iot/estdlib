#pragma once

#include "string.h"
#include "internal/fwd/string_view.h"

namespace estd {

template <class CharT,
          class Traits,
          // NOTE: Because this is marked as a 'const' string policy, resize-ish operations
          // are not as available (thus affecting remove_suffix).  Might be prudent to make
          // a special 'view' policy which is mostly const, but permits changes to size/pointer
          class Policy
          >
class basic_string_view :
    public detail::basic_string<internal::impl::allocated_array<
            layer3::allocator<const CharT, typename Policy::size_type>, Policy> >
{
    typedef detail::basic_string<internal::impl::allocated_array<
        layer3::allocator<const CharT, typename Policy::size_type>, Policy> >
        base_type;

    typedef typename base_type::allocator_type allocator_type;

    // DEBT: InitParam works, but are probably better served by something like
    // in_place_t.  InitParam was done to get around lack of variadic forwarding
    typedef typename allocator_type::InitParam init_param_t;

public:
    typedef typename base_type::size_type size_type;

    // As per spec, a no-constructor basic_string_view creates a null/null
    // scenario
    basic_string_view() : base_type(init_param_t(NULLPTR, 0)) {}

    basic_string_view(const CharT* s, size_type count) :
        base_type(init_param_t(s, count))
    {

    }

    // C-style null terminated string
    ESTD_CPP_CONSTEXPR_RET basic_string_view(const CharT* s) :
        base_type(init_param_t(s, strlen(s)))
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


typedef basic_string_view<char> string_view;


}
