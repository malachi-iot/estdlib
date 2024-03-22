#pragma once

#include "fwd.h"
#include "allocator.h"
#include "policy.h"

namespace estd {

inline namespace v0 { inline namespace avr { namespace impl {

// For "legacy" mode, this is not impl but instead entire implementation
// For newer mode, this Impl feeds estd::internal::allocated_array
// NOTE: was experimental::private_array_base
template <class T, size_t N, class Policy = estd::internal::impl::PgmPolicy<
        T, internal::impl::PgmPolicyType::String, N> >
struct pgm_array : Policy
{
    using base_type = Policy;

    using typename base_type::size_type;
    using typename base_type::const_pointer;
    using base_type::value_type;
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
    allocator_type alloc;

    constexpr const_pointer data(size_type pos = 0) const
    {
        return alloc.data(pos);
    }

    constexpr pgm_array(const_pointer data) :
        alloc(data)
    {}

    // DEBT: Just noticed AVR may indeed have true blue initializer_list support,
    // so visit that notion
    template <class ...T2>
    constexpr explicit pgm_array(T2...t) :
        alloc(in_place_t{}, t...)
    {}

    allocator_type& get_allocator() { return alloc; }
    const allocator_type& get_allocator() const { return alloc; }

    const_pointer offset(unsigned pos) const 
    {
        return alloc.data(pos);
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

    size_type size() const
    {
        return base_type::null_terminated ?
            strnlen_P(data(), 256) :
            base_type::size();
    }


#if FEATURE_ESTD_PGM_ALLOCATOR
    using accessor = internal::impl::pgm_accessor2<T>;
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