#pragma once

#include "fwd.h"
#include "allocator.h"
#include "policy.h"

namespace estd {

inline namespace v0 { inline namespace avr { namespace impl {

// For "legacy" mode, this is not impl but instead entire implementation
// For newer mode, this Impl feeds estd::internal::allocated_array
// NOTE: was experimental::private_array_base
template <class Policy>
struct pgm_array :
#if FEATURE_ESTD_PGM_ALLOCATOR
    // We prefer has-a so that we can access protected members
    Policy::allocator_traits::allocator_type,
#endif
    Policy::allocator_traits
{
    using base_type = typename Policy::allocator_traits;
    using this_type = pgm_array<Policy>;

    // So that they can see our 'data'
    friend struct estd::internal::dynamic_array_helper<this_type>;

    using typename base_type::size_type;
    using typename base_type::const_pointer;
    using typename base_type::value_type;
    //using allocator_traits = internal::impl::pgm_allocator_traits<T, N>;
    using policy_type = Policy;
    using allocator_traits = typename policy_type::allocator_traits;

    // DEBT: Dummy value so that regular estd::basic_string gets its
    // dependency satisfied
    using append_result = bool;

#if FEATURE_ESTD_PGM_ALLOCATOR
    //typedef internal::impl::pgm_allocator2<T, N> allocator_type;
    using allocator_type = typename allocator_traits::allocator_type;
    // data_ was working, but let's dogfood a bit
    //allocator_type alloc;

    allocator_type& get_allocator() { return *this; }
    constexpr const allocator_type& get_allocator() const { return *this; }

private:
    constexpr const_pointer data(size_type pos = 0) const
    {
        return allocator_type::data(pos);
    }

public:
    constexpr explicit pgm_array(const_pointer data) :
        allocator_type(data)
    {}

    // DEBT: Just noticed AVR may indeed have true blue initializer_list support,
    // so visit that notion
    template <class ...T>
    constexpr explicit pgm_array(T&&...t) :
        allocator_type(in_place_t{}, std::forward<T>(t)...)
    {}

    constexpr const_pointer offset(unsigned pos) const 
    {
        return data(pos);
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

    // DEBT: Feels like this actually belongs in dynamic_array_helper
    size_type size() const
    {
        return policy_type::size(data());
    }


#if FEATURE_ESTD_PGM_ALLOCATOR
    using accessor = internal::impl::pgm_accessor2<value_type>;
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

}}}

}