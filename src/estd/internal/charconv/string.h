#pragma once

#include "../string.h"
#include "../layer1/string.h"
#include "../../allocators/generic.h"

// EXPERIMENTAL - but promising, so keeping as internal

namespace estd { namespace internal {

// layer1 only, since layer2 or layer3 you could merely shift the pointer over in the first place

namespace impl {

template <class CharTraits, unsigned N>
class shifted_string
{
    using char_type = typename CharTraits::char_type;

    //char_type buf_[N];
    //char_type* begin_;

public:

    using policy_type = experimental::sized_string_policy<
        CharTraits, int16_t, estd::is_const<char_type>::value>;

    using value_type = char_type;
    using size_type = uint16_t;
    //using allocator_type = void;

    // DEBT: Certainly this guy already exists somewhere, I just can't remember why
    /*
    struct allocator_type
    {
        using size_type = uint16_t;
        using value_type = char_type;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using const_void_pointer = const void*;
    }; */
    //using allocator_type = nothing_allocator<value_type>;
    using allocator_type = estd::layer1::allocator<value_type, N>;

    using allocator_traits = estd::allocator_traits<allocator_type>;
    /*
    struct allocator_traits
    {
        using pointer = value_type*;
        using handle_type = pointer;
        using handle_with_offset = handle_type;
        using size_type = uint16_t;
        using allocator_valref = allocator_type;
        //using iterator = pointer;
        using iterator = locking_iterator<allocator_type, traditional_accessor<value_type> >;
    };  */

private:

    allocator_type allocator_;

public: // DEBT: Making this public
    uint8_t begin_;

public:
    shifted_string() = default;

    const value_type& lock(int pos, int count)
    {
        return allocator_.lock({}, pos + begin_, count);
    }

    const value_type& clock(int pos, int count) const
    {
        return allocator_.clock({}, pos + begin_, count);
    }

    void cunlock() const {}

    allocator_type& get_allocator() { return allocator_; }
    size_type size() const
    {
        return N - begin_;
    }

    allocator_traits::handle_with_offset offset(int offset) const
    {
        return begin_ + offset;
        //return offset;
    }
};

}

template <class CharTraits, unsigned N>
    //, class Policy =
    //experimental::sized_string_policy<
    //    CharTraits, int16_t, estd::is_const<
    //        typename CharTraits::char_type>::value> >
class shifted_string : public detail::basic_string<
    //internal::impl::allocated_array<
    //    estd::layer1::allocator<typename CharTraits::char_type, N>, Policy> >
    impl::shifted_string<CharTraits, N> >
{
    using base_type = detail::basic_string<
        impl::shifted_string<CharTraits, N> >;

public:
    ESTD_CPP_FORWARDING_CTOR(shifted_string)

    void set_begin(uint8_t v) { base_type::impl().begin_ = v; }
};

}}