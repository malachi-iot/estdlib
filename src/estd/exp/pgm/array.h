#pragma once

#include "../../internal/platform.h"

#include "impl.h"
#include "read.h"

// Shamelessly copied from https://github.com/modm-io/avr-libstdcpp/blob/master/include/initializer_list
// who themselves shamelessly copied from GCC.  Viva open source!
#include "initializer_list.h"

namespace estd {

inline namespace v0 { inline namespace avr {

// DEBT: Combine this with rest of allocated_array mechanism (dogfooding)
// yes I know this isn't really an allocated array per se...

// Just like pgm_allocator2, but from layer1 not layer2
template <class T, size_t N>
struct test_allocator : estd::layer1::allocator<const T, N>
{
    using base_type = estd::layer1::allocator<const T, N>;

    ESTD_CPP_FORWARDING_CTOR(test_allocator)

    // DEBT: See pgm_allocator2
    ESTD_CPP_CONSTEXPR_RET typename base_type::const_pointer data(
        typename base_type::size_type offset = 0) const
    {
        return base_type::data(offset);
    }
};


template <class T, unsigned N>
class test_container
{
private:
    const T data_[N];

public:
    ESTD_CPP_STD_VALUE_TYPE(T)

    // https://stackoverflow.com/questions/5549524/how-do-i-initialize-a-member-array-with-an-initializer-list

    /*
    constexpr test_container(std::initializer_list<T> l) :                                                                                                                  
        data_{l}
    {}  */
    template <typename... T2>
    constexpr test_container(T2... ts) : data_{ts...}
    {

    }

    T operator[](unsigned idx) const
    {
        return (T) internal::pgm_read<T>(data_ + (idx * sizeof(T)));
    }

#if FEATURE_ESTD_PGM_ALLOCATOR
    typedef test_allocator<T, N> allocator_type;

    using accessor = internal::impl::pgm_accessor2<T>;
    using iterator = estd::internal::locking_iterator<
        allocator_type,
        accessor,
        estd::internal::locking_iterator_modes::ro >;
#endif
};

}}

}