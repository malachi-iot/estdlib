#include "string.h"

namespace estd {

template <class CharT,
          class Traits = std::char_traits<typename estd::remove_const<CharT>::type >,
          // NOTE: Because this is marked as a 'const' string policy, resize-ish operations
          // are not as available (thus affecting remove_suffix).  Might be prudent to make
          // a special 'view' policy which is mostly const, but permits changes to size/pointer
          class Policy = experimental::sized_string_policy<Traits, int16_t, true> >
class basic_string_view :
        public basic_string<
            const CharT,
            Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
            layer3::allocator<const CharT, size_t>,
#else
            internal::single_fixedbuf_runtimesize_allocator<const CharT, size_t>,
#endif
            Policy>
{
    typedef basic_string<const CharT, Traits,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        layer3::allocator<const CharT, size_t>,
#else
        internal::single_fixedbuf_runtimesize_allocator<const CharT, size_t>,
#endif
        Policy> base_t;

    typedef typename base_t::size_type size_type;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::InitParam init_param_t;

public:
    // As per spec, a no-constructor basic_string_view creates a null/null
    // scenario
    basic_string_view() : base_t(init_param_t(NULLPTR, 0)) {}

    basic_string_view(const CharT* s, size_type count) :
        base_t(init_param_t(s, count))
    {

    }

    // C-style null terminated string
    basic_string_view(const CharT* s) :
        base_t(init_param_t(s, strlen(s)))
    {

    }


    basic_string_view(const basic_string_view& other)
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        = default;
#else
        : base_t((base_t&)other)
    {
    }
#endif


    void remove_suffix(size_type n)
    {
        // FIX: Not right - reallocate does nothing in this context
        // basic_string_view length/size hangs off max_capacity
        // of fixed allocator
        //base_t::helper.reallocate(base_t::capacity() - n);
        //base_t::helper.size(base_t::helper.size() - n);
    }
};


typedef basic_string_view<char> string_view;


}
