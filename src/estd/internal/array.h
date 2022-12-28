#pragma once

namespace estd { namespace internal {

// NOTE: Replaces the unused experimental::aligned_storage_array which itself is dependent
// on deprecated aligned_storage API

// Guidance from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1413r3.pdf

// DEBT: May not need to be layer1 explicitly, it seems span can cover the job of layer2 and layer3
template <class T, unsigned N>
struct uninitialized_array
{
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef unsigned size_type;

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

    ESTD_CPP_CONSTEXPR_RET bool empty() const { return N == 0; }
    ESTD_CPP_CONSTEXPR_RET size_type size() const { return N; }
    ESTD_CPP_CONSTEXPR_RET size_type max_size() const { return N; }

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

template <class TBase>
struct array_base2 : TBase
{
    typedef TBase base_type;
    typedef typename base_type::pointer pointer;
    typedef typename base_type::const_pointer const_pointer;
    typedef typename base_type::pointer iterator;
    typedef typename base_type::value_type& reference;
    typedef const typename base_type::value_type& const_reference;
    typedef typename base_type::size_type size_type;

    pointer begin() { return base_type::data(); }
    ESTD_CPP_CONSTEXPR_RET const_pointer begin() const { return base_type::data(); }

    reference front() { return *begin(); }
    ESTD_CPP_CONSTEXPR_RET const_reference front() const { return *begin(); }

    reference back() { return *base_type::end(); }
    ESTD_CPP_CONSTEXPR_RET const_reference back() const { return *base_type::end(); }

    reference operator[](size_type pos) { return *base_type::get_at(pos); }
    ESTD_CPP_CONSTEXPR_RET const_reference operator[](size_type pos) const
    {
        return *base_type::get_at(pos);
    }
};

// EXPERIMENTAL
template <class T, unsigned N>
struct layer1_allocator
{
    array_base2<uninitialized_array<T, N> > storage;
};

}

// EXPERIMENTAL
template <class T, unsigned N>
struct allocator_traits<internal::layer1_allocator<T, N> >
{

};

}