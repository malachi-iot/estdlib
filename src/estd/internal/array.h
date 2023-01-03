#pragma once

#include "fwd/array.h"
#include "type_traits.h"

namespace estd { namespace internal {

// NOTE: Replaces the unused experimental::aligned_storage_array which itself is dependent
// on deprecated aligned_storage API

// Guidance from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1413r3.pdf and pggcc-29
// Turns out alignment isn't a critical issue here

namespace impl {

template <unsigned N>
struct array_base_size
{
    typedef unsigned size_type;

    ESTD_CPP_CONSTEXPR_RET bool empty() const { return N == 0; }
    ESTD_CPP_CONSTEXPR_RET size_type size() const { return N; }
    ESTD_CPP_CONSTEXPR_RET size_type max_size() const { return N; }
};


// DEBT: Move this part to impl area
template <class T, unsigned N>
struct uninitialized_array : array_base_size<N>
{
    typedef array_base_size<N> base_type;
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef typename base_type::size_type size_type;

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

#if __cplusplus >= 201703L
    // Not yet tested
    //constexpr
#endif
    pointer data()
    {
        return reinterpret_cast<pointer>(data_);
    }

    ESTD_CPP_CONSTEXPR_RET const_pointer data() const
    {
        return reinterpret_cast<const_pointer>(data_);
    }

    pointer end() { return data() + N; }
    ESTD_CPP_CONSTEXPR_RET const_pointer end() const { return data() + N; }

protected:
    // Making these internal/protected APIs so that we can dogfood and really test our
    // alignment with the [] operator
    pointer get_at(size_type pos)
    {
        return reinterpret_cast<pointer>(data_[pos].data);
    }

    ESTD_CPP_CONSTEXPR_RET const_pointer get_at(size_type pos) const
    {
        return reinterpret_cast<const_pointer>(data_[pos].data);
    }
};

template <class T, unsigned N>
struct traditional_array : array_base_size<N>
{
    typedef array_base_size<N> base_type;
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef typename base_type::size_type size_type;

    T data_[N];

    pointer data() { return data_; }
    ESTD_CPP_CONSTEXPR_RET const_pointer data() const { return data_; }

    pointer end() { return data() + N; }
    ESTD_CPP_CONSTEXPR_RET const_pointer end() const { return data() + N; }

protected:
    // Making these internal/protected APIs so that we can dogfood and really test our
    // alignment with the [] operator
    pointer get_at(size_type pos) { return data() + pos; }
    ESTD_CPP_CONSTEXPR_RET const_pointer get_at(size_type pos) const
    {
        return data() + pos;
    }

    ESTD_CPP_DEFAULT_CTOR(traditional_array)

#ifdef FEATURE_CPP_INITIALIZER_LIST
    constexpr traditional_array(::std::initializer_list<value_type> init) : data_(init)
    {

    }
#endif
};

}

template <class TBase>
struct array_base2 : TBase
{
    typedef TBase base_type;

    typedef typename base_type::pointer pointer;
    typedef typename base_type::const_pointer const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef typename base_type::value_type& reference;
    typedef const typename base_type::value_type& const_reference;
    typedef typename base_type::size_type size_type;

    iterator begin() { return base_type::data(); }
    ESTD_CPP_CONSTEXPR_RET const_iterator begin() const { return base_type::data(); }

    reference front() { return *begin(); }
    ESTD_CPP_CONSTEXPR_RET const_reference front() const { return *begin(); }

    reference back() { return *base_type::end(); }
    ESTD_CPP_CONSTEXPR_RET const_reference back() const { return *base_type::end(); }

    reference operator[](size_type pos) { return *base_type::get_at(pos); }
    ESTD_CPP_CONSTEXPR_RET const_reference operator[](size_type pos) const
    {
        return *base_type::get_at(pos);
    }

    ESTD_CPP_FORWARDING_CTOR(array_base2)
};

// EXPERIMENTAL
template <class T, unsigned N>
struct layer1_allocator
{
    array_base2<impl::uninitialized_array<T, N> > storage;
};

// Just like regular std::array except T constructor is not called
template <class T, unsigned N, class TBase =
        typename estd::conditional<
            estd::is_integral<T>::value,
            estd::array<T, N>,
            typename estd::internal::array_base2<impl::uninitialized_array<T, N> > >
            ::type >
struct uninitialized_array : TBase {};


}

// EXPERIMENTAL
template <class T, unsigned N>
struct allocator_traits<internal::layer1_allocator<T, N> >
{

};

template<class T, std::size_t N, typename TSize>
struct array : public internal::array_base2<internal::impl::traditional_array<T, N> >
{
    typedef internal::array_base2<internal::impl::traditional_array<T, N> > base_type;

    ESTD_CPP_FORWARDING_CTOR(array)
};

}