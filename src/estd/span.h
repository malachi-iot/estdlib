#pragma once

// In the past we used a variant of array coupled with allocated buffer helpers,
// but really spans are very much not allocated.  So "redoing" that work here,
// which turns out to be slightly less code and easier to understand.

#include "type_traits.h"
#include "cstddef.h"

namespace estd {

namespace detail {

typedef estd::integral_constant<estd::size_t, (estd::size_t)-1> dynamic_extent;

}

template <class T, estd::size_t Extent = detail::dynamic_extent::value>
class span;

#ifdef FEATURE_CPP_INLINE_VARIABLES
inline CONSTEXPR ptrdiff_t dynamic_extent = detail::dynamic_extent::value;
#endif

namespace internal {

template <class T>
struct span_base
{
    // NOTE: this is for convenience, but does typedef value_type wrong
    ESTD_CPP_STD_VALUE_TYPE(T)

    typedef estd::size_t size_type;
    using iterator = pointer;
    using const_iterator = const_pointer;

protected:
    T* const data_;

    constexpr span_base(pointer data = nullptr) : data_(data) {}

public:
    template <estd::size_t count>
    constexpr span<T, count> first() const
    {
        return data_;
    }

    constexpr span<T> first(size_type count) const
    {
        return span<T>(data_, count);
    }

    constexpr pointer data() const { return data_; }
    constexpr iterator begin() const { return data_; }
    constexpr const_iterator cbegin() const { return data_; }

    constexpr const_reference operator[](size_type idx) const
    {
        return data_[idx];
    }

    ESTD_CPP_CONSTEXPR(14) reference operator[](size_type idx)
    {
        return data_[idx];
    }
};

// Compile time size
template <class T, estd::size_t N = detail::dynamic_extent::value>
struct span : span_base<T>
{
    typedef span_base<T> base_type;

    typedef typename base_type::size_type size_type;
    typedef typename base_type::pointer pointer;
    typedef T element_type;

    static ESTD_CPP_CONSTEXPR_RET size_type size() { return N; }

    ESTD_CPP_CONSTEXPR_RET span(pointer data) : base_type(data) {}
};

// Runtime size
template <class T>
struct span<T, detail::dynamic_extent::value> : span_base<T>
{
    using base_type = span_base<T>;

    typedef typename base_type::size_type size_type;
    typedef typename base_type::pointer pointer;
    typedef T element_type;

    const size_type size_;

    constexpr size_type size() const { return size_; }

    constexpr span() : size_{0}    {}
    constexpr span(pointer data, size_type size) : base_type(data),
        size_(size) {}

    constexpr span(const span& copy_from) :
        base_type(copy_from.data()), size_(copy_from.size())
    {}

    template <int N2>
    constexpr span(element_type (&data) [N2]) :
        base_type(data), size_(N2)
    {}
};

}

template <class T, estd::size_t Extent>
class span : public internal::span<T, Extent>
{
    using this_type = span<T, Extent>;
    using base_type = internal::span<T, Extent>;

    static CONSTEXPR bool is_dynamic = Extent == detail::dynamic_extent::value;

public:
    using typename base_type::size_type;
    using typename base_type::pointer;
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using base_type::size;

    using index_type = size_type;

    static constexpr ptrdiff_t extent = Extent;

    typedef T element_type;
    typedef typename estd::remove_cv<T>::type value_type;

    ESTD_CPP_CONSTEXPR_RET index_type size_bytes() const
    { return size() * sizeof(element_type); }

    constexpr bool empty() const NOEXCEPT
    {
        return size() == 0;
    }

    constexpr iterator end() const { return base_type::data_ + size(); }
    constexpr const_iterator cend() const { return base_type::data_ + size(); }

    // DEBT:
    // "This overload participates in overload resolution only if extent == 0 || extent == std::dynamic_extent."
    span() = default;

#ifdef FEATURE_CPP_DEFAULT_TARGS
    // ExtendLocal needed because SFINAE function selection needs that
    // fluidity
    // dynamic flavor
    // DEBT: add c++20 'explicit' version
    template <estd::size_t N, estd::size_t ExtentLocal = Extent,
        class ExtentOnly = enable_if_t<ExtentLocal == detail::dynamic_extent::value>>
    constexpr span(element_type (&data) [N]) : base_type(data, N) {}

    // DEBT: Really we need to take in template <class In, class End>
    template <estd::size_t ExtentLocal = Extent,
        class ExtentOnly = enable_if_t<ExtentLocal == detail::dynamic_extent::value>>
    constexpr span(T* first, T* last) : base_type(first, last - first) {}

    template <estd::size_t ExtentLocal = Extent,
        class ExtentOnly = enable_if_t<ExtentLocal == detail::dynamic_extent::value>>
    constexpr span(T* first, size_type count) : base_type(first, count) {}

    // constant size flavor
    template <estd::size_t N, estd::size_t ExtentLocal = Extent,
        class ExtentOnly = enable_if_t<ExtentLocal == N>>
    constexpr span(element_type (&data) [N], bool = true) : base_type(data) {}

    template <estd::size_t N, estd::size_t ExtentLocal = Extent,
        class ExtentOnly = typename enable_if<ExtentLocal == detail::dynamic_extent::value>::type>
    constexpr span(const estd::span<T, N>& other) :
        base_type(other.data(), N) {}
#else
    // Only works with dynamic extend mode
    template <estd::size_t N>
    span(element_type (&data)) : base_type(data, N) {}

    span(pointer data, index_type count) :
            base_type(data, count) {}
#endif

    // most definitely a 'shallow clone'
    // utilizing 'base_type' enables clever init constructors of compatible base to
    // widen field of how this span can initialize
    ESTD_CPP_CONSTEXPR_RET span(const base_type& copy_from) : base_type(copy_from) {}
    // DEBT: This extra constructor is needed to quiet down deprecation warnings
    // about implicitly defined constructor - apparently above base_type flavor
    // doesn't fill all the roles
    ESTD_CPP_CONSTEXPR_RET span(const span& copy_from) : base_type(copy_from) {}

    span& operator=(const span& copy_from)
    {
        return * new (this) span(copy_from);
    }

    span<element_type> subspan(size_type offset, size_type count = detail::dynamic_extent())
    {
        if(count == detail::dynamic_extent::value) count = base_type::size();

        pointer data = base_type::data();

        return span<element_type>(data + offset, count);
    }
};


/// @deprecated - use span instead
typedef span<const uint8_t> const_buffer;
typedef span<uint8_t> mutable_buffer;

template <class T, estd::size_t N, estd::size_t  S>
span<const byte, S> as_bytes(span<T, N> s) NOEXCEPT;

template <class T>
ESTD_CPP_CONSTEXPR_RET span<const byte, detail::dynamic_extent::value> as_bytes(span<T, detail::dynamic_extent::value> s) NOEXCEPT
{
    return span<const byte, detail::dynamic_extent::value>(reinterpret_cast<const byte*>(s.data()), s.size_bytes());
}


template <class T, estd::size_t  N>
ESTD_CPP_CONSTEXPR_RET span<const byte, N * sizeof(T)> as_bytes(span<T, N> s) NOEXCEPT
{
    return span<const byte, N * sizeof(T)>(reinterpret_cast<const byte*>(s.data()));
}


// UNTESTED
template <class T, estd::size_t N, estd::size_t S>
span<const byte, S> as_writable_bytes(span<T, N> s) NOEXCEPT;


template <class T>
span<byte, detail::dynamic_extent::value> as_writable_bytes(span<T, detail::dynamic_extent::value> s) NOEXCEPT
{
    return span<byte, detail::dynamic_extent::value>(reinterpret_cast<byte*>(s.data()), s.size_bytes());
}


template <class T, estd::size_t N>
span<byte, N * sizeof(T)> as_writable_bytes(span<T, N> s) NOEXCEPT
{
    return span<byte, N * sizeof(T)>(reinterpret_cast<byte*>(s.data()));
}

}
