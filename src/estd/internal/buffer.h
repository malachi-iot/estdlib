#pragma once

#include "../array.h"
#include "../type_traits.h"

namespace estd {

namespace internal {

// kind of a reinterpretation of boost::mutable_buffer
// void* idea is great, but nearly all my use cases which aren't served by estd::array or estd::vector
// call for a simplistic uint8_t/byte buffer
// TODO: Once estd::array has been turned into something with an underlying (fixed) allocator,
// upgrade these too.  That way these can smoothly participate in locking/unlocking rather than
// demanding pointers be flattened out as they are now

namespace layer1 {

// remember default estd/std array implementation conforms to our layer1 strategy
template <size_t size>
class mutable_buffer : public estd::array<uint8_t, size>
{
    typedef estd::array<uint8_t, size> base_t;
    typedef typename base_t::value_type value_type;

public:
#ifdef FEATURE_CPP_INITIALIZER_LIST
    mutable_buffer(std::initializer_list<value_type> init) : base_t(init)
    {

    }
#endif
};

}

namespace layer2 {

template <size_t size>
class mutable_buffer : public estd::layer2::array<uint8_t, size>
{
    typedef estd::layer2::array<uint8_t, size> base_t;

public:
#ifdef FEATURE_CPP_DEFAULT_TARGS
    // NOTE: 90% sure this isn't gonna work < c++11
    template <class TPtr = uint8_t*, typename =
              typename estd::enable_if<!estd::is_array<TPtr>::value>::type >
    mutable_buffer(TPtr data) : base_t(data) {}

    // NOTE: sorta works, but uint8_t* data constructor always wins if this fails
    template <size_t N, typename = typename estd::enable_if<N >= size>::type>
    mutable_buffer(uint8_t (&data) [N]) : base_t(data) {}
#else
    mutable_buffer(uint8_t* data) : base_t(data) {}
#endif
};

}

namespace layer3 {

// TODO: once we lean array itself on allocated_array, beef up constructors here
// NOTE: being that this is the underlying code for span, we conform this to
// span's signatures (index_type, for example instead of size_type)
template <class T, class TSize = size_t>
class buffer : public estd::layer3::array<T, size_t>
{
protected:
    typedef estd::layer3::array<T, size_t> base_t;

public:
    typedef T element_type;
    typedef typename base_t::pointer pointer;
    typedef typename base_t::size_type index_type;
    typedef typename estd::remove_cv<T>::type value_type;

    // This is a low level call, but buffers are low level creatures
    // gently discouraged during mutable_buffer,
    // strongly discouraged during const_buffer,
    // but not necessarily wrong to use it
    void resize(index_type n) { base_t::m_size = n; }

    CONSTEXPR index_type size_bytes() const
    { return base_t::size() * sizeof(element_type); }

    buffer(pointer data, index_type size) :
            base_t(data, size) {}

    template <size_t N>
    buffer(element_type (&data) [N]) : base_t(data, N) {}

    // most definitely a 'shallow clone'
    buffer(const base_t& clone_from) :
            base_t(clone_from.data(), clone_from.size()) {}
};

}

}

}
