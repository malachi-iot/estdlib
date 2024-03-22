#pragma once

#include "../../internal/platform.h"

#include "impl.h"
#include "read.h"

// Shamelessly copied from https://github.com/modm-io/avr-libstdcpp/blob/master/include/initializer_list
// who themselves shamelessly copied from GCC.  Viva open source!
//#include "initializer_list.h"

#include "../../internal/macro/push.h"

namespace estd {

namespace internal {

// DEBT: Try making one that comes from 'Core' policy type that
// the rest can derive from... a bit squirrely
template <typename T, impl::PgmPolicyType type_, unsigned N>
struct dynamic_array_helper<avr::impl::pgm_array<
    impl::PgmPolicy<T, type_, N> > >
{
    using impl_type = avr::impl::pgm_array<impl::PgmInlinePolicy<T, N>>;

    typedef internal::dynamic_array<impl_type> dynamic_array;
    typedef internal::allocated_array<impl_type> array;

    typedef typename array::value_type value_type;
    typedef typename array::pointer pointer;
    typedef typename array::const_pointer const_pointer;
    typedef typename array::size_type size_type;

    static size_type copy_to(const array& a,
        typename estd::remove_const<value_type>::type* dest,
        size_type count, size_type pos = 0)
    {
        const size_type _end = estd::min(count, a.size());
        // DEBT: Not 100% convinced that resolves
        // back to a pointer smoothly or at all, even though compiles OK
        //const_pointer src = a.ofset(pos);
        
        // DEBT: A tad TOO friendly about allocated_array internals
        const_pointer src = a.m_impl.data(pos);
        memcpy_P(dest, src, _end * sizeof(value_type));
        return _end;
    }
};

}

inline namespace v0 { inline namespace avr {


// DEBT: This guy works, but pgm_array is better
template <class T, unsigned N>
class test_container : //protected estd::layer1::allocator<T, N>
    protected estd::internal::impl::layer1_pgm_allocator<T, N>
{
    //using base_type = estd::layer1::allocator<T, N>;
    using base_type = estd::internal::impl::layer1_pgm_allocator<T, N>;

private:
    //const T data_[N];

public:
    ESTD_CPP_STD_VALUE_TYPE(T)

    // https://stackoverflow.com/questions/5549524/how-do-i-initialize-a-member-array-with-an-initializer-list

    /*
    constexpr test_container(std::initializer_list<T> l) :                                                                                                                  
        data_{l}
    {}  */
    template <typename... T2>
    constexpr test_container(T2... ts) :
        //data_{ts...}
        base_type(in_place_t{}, ts...)
    {

    }

    T operator[](unsigned idx) const
    {
        //return (T) internal::pgm_read<T>(data_ + (idx * sizeof(T)));
        return (T) internal::pgm_read<T>(base_type::buffer + (idx * sizeof(T)));
    }

#if FEATURE_ESTD_PGM_ALLOCATOR
    typedef estd::internal::impl::layer1_pgm_allocator<T, N> allocator_type;

    using accessor = internal::impl::pgm_accessor2<T>;
    using iterator = estd::internal::locking_iterator<
        allocator_type,
        accessor,
        estd::internal::locking_iterator_modes::ro >;

    iterator begin() const
    {
        accessor a{base_type::buffer};
        return iterator(a);
    }

    iterator end() const
    {
        accessor a{base_type::buffer + N};
        return iterator(a);
    }
#endif
};

// It turns out std::initializer_list seems to be present for AVR and tries
// to take over in allocated_array, so brute force things
// NOTE: Hmm maybe that was a red herring, now it less confusingly complains
// about allocated_array's protected constructor
#define FEATURE_ESTD_PGM_EXP_IL 0

#if FEATURE_ESTD_PGM_EXP_IL
template <class T, unsigned N>
using pgm_array = estd::internal::allocated_array<
    avr::impl::pgm_array<internal::impl::PgmInlinePolicy<T, N> > >;
#else
template <class T, unsigned N>
struct pgm_array : estd::internal::allocated_array<
    avr::impl::pgm_array<internal::impl::PgmInlinePolicy<T, N> > >
{
    using base_type = estd::internal::allocated_array<
    avr::impl::pgm_array<internal::impl::PgmInlinePolicy<T, N> > >;

    template <class ...T2>
    constexpr pgm_array(T2&&...t) :
        base_type(std::forward<T2>(t)...) {}
};
#endif

}}

}

#include "../../internal/macro/pop.h"
