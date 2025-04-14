#pragma once

#include "fwd/array.h"
#include "type_traits.h"
#include "../algorithm.h"
#include "fwd/memory.h"
#include "iterator_standalone.h"

namespace estd { namespace internal {

// Guidance from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1413r3.pdf and pggcc-29
// Turns out alignment isn't a critical issue here

namespace impl {

// 14APR25 MB DEBT: I forgot the point of deviating the size type here.  For constexpr world, feels useless
template <size_t N, typename Size = typename internal::deduce_fixed_size_t<N>::size_type>
struct array_base_size
{
    typedef Size size_type;

    static ESTD_CPP_CONSTEVAL bool empty() { return N == 0; }
    static ESTD_CPP_CONSTEVAL size_type size() { return N; }
    static ESTD_CPP_CONSTEVAL size_type max_size() { return N; }
};


template <class T, size_t N>
struct uninitialized_array : array_base_size<N>
{
    using base_type = array_base_size<N>;
    using typename base_type::size_type;

    ESTD_CPP_STD_VALUE_TYPE(T)

    struct container
    {
        // as per pggcc-29, looks like arrays don't have to worry much about alignment
#if UNUSED
#ifdef FEATURE_CPP_ALIGN
        alignas(T)
#elif defined(__GNUC__)
        // FIX: This gets mad for odd sizes
        __attribute__((aligned(sizeof(T))))
#endif
#endif
        estd::byte data[sizeof(T)];
    };

    container data_[N];

    ESTD_CPP_CONSTEXPR(14) pointer data()
    {
        return reinterpret_cast<pointer>(data_);
    }

    constexpr const_pointer data() const
    {
        return reinterpret_cast<const_pointer>(data_);
    }

protected:
    // Making these internal/protected APIs so that we can dogfood and really test our
    // alignment with the [] operator
    ESTD_CPP_CONSTEXPR(14) pointer get_at(size_type pos)
    {
        return reinterpret_cast<pointer>(data_[pos].data);
    }

    constexpr const_pointer get_at(size_type pos) const
    {
        return reinterpret_cast<const_pointer>(data_[pos].data);
    }
};

template <class T, size_t N>
struct traditional_array : array_base_size<N>
{
    typedef array_base_size<N> base_type;

    ESTD_CPP_STD_VALUE_TYPE(T)

    typedef typename base_type::size_type size_type;

    T data_[N];

    ESTD_CPP_CONSTEXPR(14) pointer data() { return data_; }
    constexpr const_pointer data() const { return data_; }

protected:
    // Making these internal/protected APIs so that we can dogfood and really test our
    // alignment with the [] operator
    ESTD_CPP_CONSTEXPR(14) pointer get_at(size_type pos) { return data() + pos; }
    constexpr const_pointer get_at(size_type pos) const
    {
        return data() + pos;
    }

    ESTD_CPP_DEFAULT_CTOR(traditional_array)

#ifdef __cpp_initializer_lists
    inline traditional_array(::std::initializer_list<value_type> init)
    {
        estd::copy(init.begin(), init.end(), data());
    }
#endif
};

}

template <class Impl>
struct array : Impl
{
    using base_type = Impl;
    using base_type::data;
    using base_type::max_size;

    using typename base_type::pointer;
    using typename base_type::const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::reference reference;
    typedef const typename base_type::value_type& const_reference;
    typedef typename base_type::size_type size_type;

    iterator begin() { return data(); }
    constexpr const_iterator begin() const { return data(); }
    constexpr const_iterator cbegin() const { return data(); }

    pointer end() { return data() + max_size(); }
    constexpr const_iterator end() const { return data() + max_size(); }
    constexpr const_iterator cend() const { return data() + max_size(); }


    reference front() { return *begin(); }
    constexpr const_reference front() const { return *begin(); }

    reference back() { return *base_type::end(); }
    constexpr const_reference back() const { return *base_type::end(); }

    reference operator[](size_type pos) { return *base_type::get_at(pos); }
    constexpr const_reference operator[](size_type pos) const
    {
        return *base_type::get_at(pos);
    }

    void fill(const_reference value)
    {
        estd::fill_n(begin(), base_type::size(), value);
    }

    ESTD_CPP_FORWARDING_CTOR(array)
    ESTD_CPP_FORWARDING_CTOR_LIST(value_type, array)

};

// EXPERIMENTAL
template <class T, unsigned N>
struct layer1_allocator
{
    array_base2<impl::uninitialized_array<T, N> > storage;
};

// Just like regular std::array except T constructor is not called
// NOTE: We flow through with Base and fall back to regular estd::array if dealing
// with integral types, since they have no constructor and it makes debugging easier and
// potentially the codebase slightly smaller since the optimizer works less hard
template <class T, unsigned N>
using uninitialized_array = conditional_t<
    is_integral<T>::value,
    estd::array<T, N>,
    internal::array<impl::uninitialized_array<T, N>>>;


}

// EXPERIMENTAL
template <class T, unsigned N>
struct allocator_traits<internal::layer1_allocator<T, N> >
{

};

}
