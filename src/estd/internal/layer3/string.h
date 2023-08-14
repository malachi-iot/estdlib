#pragma once

#include "../fwd/string.h"
#include "../../policy/string.h"

namespace estd { namespace layer3 {

template<class CharT, bool null_terminated = true,
         class Traits = estd::char_traits<typename estd::remove_const<CharT>::type >,
         class Policy = typename estd::conditional<null_terminated,
                experimental::null_terminated_string_policy<Traits, int16_t, estd::is_const<CharT>::value>,
                experimental::sized_string_policy<Traits, int16_t, estd::is_const<CharT>::value> >::type>
class basic_string
        : public estd::basic_string<
                CharT, Traits,
// FIX: Not ready yet, because layer3::allocator constructor and class InitParam doesn't fully
// initialize underlying allocator
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                estd::layer3::allocator<CharT, typename Policy::size_type>,
#else
                estd::internal::single_fixedbuf_runtimesize_allocator < CharT >,
#endif
                Policy>
{
    typedef estd::basic_string<
            CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
            estd::layer3::allocator<CharT, typename Policy::size_type>,
#else
            estd::internal::single_fixedbuf_runtimesize_allocator < CharT >,
#endif
            Policy>
            base_t;

    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::impl_type helper_type;

public:
    typedef typename base_t::size_type size_type;

protected:
    typedef typename allocator_type::InitParam init_t;

    // certain varieties (such as basic_string_view and layer3::const_string) only have one size, the initial
    // buffer size
    basic_string(CharT* buffer, size_type buffer_size, bool) :
        base_t(init_t(buffer, buffer_size))
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
    basic_string(CharT (&buffer) [N], size_type initial_size = -1) :
        base_t(init_t(buffer, N))
    {
        if(initial_size == -1)
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

    template <class ForeignAllocator, class ForeignTraits>
    basic_string(const estd::basic_string<CharT, Traits, ForeignAllocator, ForeignTraits> & copy_from)
        // FIX: very bad -- don't leave things locked!
        // only doing this because we often pass around layer1, layer2, layer3 strings who
        // don't care about lock/unlock
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
    }


    template <class ForeignAllocator, class ForeignTraits>
    basic_string& operator=(const estd::basic_string<CharT, Traits, ForeignAllocator, ForeignTraits>& copy_from)
    {
        base_t::operator =(copy_from);
        return *this;
    }
};


// defaults to null-terminated variety
typedef basic_string<char> string;



}}
