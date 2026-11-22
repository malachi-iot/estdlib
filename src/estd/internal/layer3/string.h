#pragma once

#include "../fwd/string.h"
#include "../fwd/string_view.h"
#include "../../policy/string.h"
#include "../../allocators/fixed.h"
#include "../string.h"
#include "../string/layer.h"
#include "../../span.h"

#if FEATURE_STD_STRING_VIEW
#include <string_view>
#endif

namespace estd { namespace layer3 {

// NOTE: Slowly phasing this guy out in favor of basic_string_view.
// Keep in mind though he supports a size 0 (infinite) for null terminated strings.  Also
// he differs in that '=' does a deep copy
// 22AUG26 MB DEBT: Perhaps we ought to take -1 for a null terminated string so that 0 can mate to nullptr data
template<class CharT, bool null_terminated = true,
         class Traits = estd::char_traits<estd::remove_const_t<CharT>>,
         class Policy = internal::string_policy_helper<CharT, Traits,
            null_terminated ? internal::string_options::null_terminated : internal::string_options::none,
            uint16_t>>
class basic_string :
    public estd::internal::basic_string<
        estd::layer3::allocator<CharT, typename Policy::size_type>,
        Policy>,
    public estd::internal::mixins::string<basic_string<CharT, null_terminated, Traits, Policy>, Traits>
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
    using base_type::data;

protected:
    using span_type = span<CharT>;

    // certain varieties (such as basic_string_view and layer3::const_string) only have one size, the initial
    // buffer size
    constexpr basic_string(CharT* buffer, size_type buffer_size, bool) :
        base_type(span_type(buffer, buffer_size))
    {
    }

public:
#if EXP
    template <size_type N>
    basic_string(const CharT (&buffer) [N]) : base_type(span<CharT, N>(buffer))
    { }
#endif

    /// Pass in full backing storage (fixed size) then also a clue about initial size
    /// @param initial_size - explicit size of string, or npos (default) to do a strlen
    template <size_type N>
    basic_string(CharT (&buffer) [N], size_type initial_size = base_type::npos) :
        base_t(span<CharT, N>(buffer))
    {
        if(initial_size == base_type::npos)
            initial_size = strlen(buffer);

        // TODO: Enable for a string-mode version
        // FIX: Causes issues for (null terminated+fixed size) flavor
        //base_t::ensure_total_size(initial_size);
        base_t::impl().size(initial_size);
    }

    /*
     * 26JUL26 MB - Disabling this perfectly functional constructor.  Reason being
     * buffer_size really ought to have come first.  So, use the new below
     * basic_string(start, end) as a workaround.  I may just sunset this guy forever,
     * it's easy to get confused with those two size parameters.
    basic_string(CharT* buffer, size_type initial_size, size_type buffer_size) :
        base_t(init_t(buffer, buffer_size))
    {
        base_t::impl().size(initial_size);
    }   */

    ESTD_CPP_CONSTEXPR(14) basic_string(CharT* start, CharT* end, size_type initial_size) :
        base_type(span_type(start, end))
    {
        base_t::impl().size(initial_size);
    }

    // DEBT: This whole thing needs to filter by non-locking allocator
    template <class Impl>
    basic_string(const estd::detail::basic_string<Impl>& copy_from)
        // DEBT: Precision loss here generates warnings and rightly so.  This whole
        // init mechanism needs a rework - or a deprecation completely, since layer3::basic_string
        // is more or less a basic_string_view
        : base_type(span_type(copy_from.clock(), copy_from.max_size()))
    {
        static_assert(is_const<typename Impl::value_type>::value, "Only const supported at this time");
        static_assert(helper_type::uses_termination(), "Only supports null terminated at this time");

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
};


// defaults to null-terminated variety
using string = basic_string<char>;



}}
