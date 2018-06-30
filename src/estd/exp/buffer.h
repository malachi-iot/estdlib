#pragma once

#include "../array.h"
#include "../span.h"
#include "../type_traits.h"

namespace estd {

namespace experimental {

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
    template <size_t N, typename = typename std::enable_if<N >= size>::type>
    mutable_buffer(uint8_t (&data) [N]) : base_t(data) {}
#else
    mutable_buffer(uint8_t* data) : base_t(data) {}
#endif
};

}

namespace layer3 {


typedef span<uint8_t> mutable_buffer;
typedef span<const uint8_t> const_buffer;


}

// default implementations are of the layer3 variety (buf* & size member field)
typedef layer3::mutable_buffer mutable_buffer;
typedef layer3::const_buffer const_buffer;

}

// this one in particular is getting some use and performing well
typedef experimental::const_buffer const_buffer;

}
