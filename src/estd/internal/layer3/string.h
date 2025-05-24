#pragma once

#include "../fwd/string.h"
#include "../fwd/string_view.h"
#include "../../policy/string.h"
#include "../../allocators/fixed.h"
#include "../string.h"

namespace estd { namespace layer3 {

// NOTE: Slowly phasing this guy out in favor of basic_string_view.
// Keep in mind though he supports a size 0 (infinite) for null terminated strings.  Also
// he differs in that '=' does a deep copy
template<class CharT, bool null_terminated = true,
         class Traits = estd::char_traits<estd::remove_const_t<CharT>>,
         class Policy = conditional_t<null_terminated,
                internal::null_terminated_string_policy<Traits, uint16_t, estd::is_const<CharT>::value>,
                internal::sized_string_policy<Traits, uint16_t, estd::is_const<CharT>::value> >>
class basic_string : public estd::internal::basic_string<
                estd::layer3::allocator<CharT, typename Policy::size_type>,
                Policy>
{
    using base_type = estd::internal::basic_string<
        estd::layer3::allocator<CharT, typename Policy::size_type>,
        Policy>;

    using base_t = base_type;
    typedef typename base_t::allocator_type allocator_type;
    // DEBT: Change helper_type name to impl_type if it isn't aligned with new (also poorly named)
    // string optimization "helper"
    typedef typename base_t::impl_type helper_type;

public:
    using typename base_type::size_type;
    using typename base_type::view_type;
    using base_type::data;

protected:
    typedef typename allocator_type::InitParam init_t;

    // certain varieties (such as basic_string_view and layer3::const_string) only have one size, the initial
    // buffer size
    constexpr basic_string(CharT* buffer, size_type buffer_size, bool) :
        base_type(init_t(buffer, buffer_size))
    {
    }

public:
    /*
    template <size_type N>
    basic_string(const CharT (&buffer) [N], bool source_null_terminated = true) :
        base_t(typename allocator_type::InitParam(buffer, N))
    {
        base_t::helper.size(N - (source_null_terminated ? 1 : 0));
    } */

    template <size_type N>
    basic_string(CharT (&buffer) [N], size_type initial_size = base_type::npos) :
        base_t(init_t(buffer, N))
    {
        if(initial_size == base_type::npos)
            initial_size = strlen(buffer);

        // TODO: Enable for a string-mode version
        //base_t::ensure_total_size(initial_size);
        base_t::impl().size(initial_size);
    }

    basic_string(CharT* buffer, size_type initial_size, size_type buffer_size) :
        base_t(init_t(buffer, buffer_size))
    {
        base_t::impl().size(initial_size);
    }

    template <class Impl>
    basic_string(const estd::detail::basic_string<Impl>& copy_from)
        // DEBT: Precision loss here generates warnings and rightly so.  This whole
        // init mechanism needs a rework - or a deprecation completely, since layer3::basic_string
        // is more or less a basic_string_view
        : base_t(init_t(copy_from.clock(), copy_from.max_size()))
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert(helper_type::uses_termination(), "Only supports null terminated at this time");
#endif

        // can't do following right now since compiler still tries to compile null
        // terminated flavor for this, which will fail when using const char
        /*
        // If null terminated, don't set size as
        // a) it's already set in the incoming buffer
        // b) we'd be modifying original buffer - though in theory just rewriting
        //    0 back onto the existing 0
        if(!impl_type::uses_termination())
        {
            base_t::helper.size(copy_from.size());
        } */
        copy_from.cunlock();
    }


    // DEBT: Above copy constructor ought to handle this, but we have a const/InitParam issue
    // holding us back
    template <class Impl>
    basic_string& operator=(const estd::detail::basic_string<Impl>& copy_from)
    {
        base_type::operator =(copy_from);
        return *this;
    }

    // A little clumsy since basic_string_view treats everything as const already,
    // so if we are converting from a const_string we have to remove const from CharT
    //typedef basic_string_view<typename estd::remove_const<CharT>::type, Traits> view_type;

    constexpr operator view_type() const
    {
        return { data(), typename view_type::size_type(base_type::size()) };
    }
};


// defaults to null-terminated variety
typedef basic_string<char> string;



}}
