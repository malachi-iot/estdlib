#pragma once

extern "C" {
#include "../../ext/willemt/bipbuffer/bipbuffer.h"
}

#include "../../cstddef.h"

// NOTE: platformio gcc is currently at 7.3.0 which appears to have the glitch:
// https://stackoverflow.com/questions/45047888/suppress-wlto-type-mismatch-warning-for-struct-with-flexible-array
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81440
// Since only the .c file truly touches the flexible array, I believe this does not impede functionality.

#if __cpp_concepts
namespace estd {
namespace concepts { inline namespace v1 {

template <class T>
concept Bipbuf = requires(T& b, unsigned char* data)
{
    b.peek(0);
    b.offer(data, 0);
    b.offer_begin();
    b.offer_end(0);
#if __cpp_lib_concepts
    { b.used() } -> std::convertible_to<unsigned>;
    { b.unused() } -> std::convertible_to<unsigned>;
#endif
};

}}
}
#endif

namespace estd { namespace internal {

// DEBT: Move these to a cpp or hpp somewhere
// more or less copy/pasting & splitting out what bipbuf_offer does

inline void check_for_switch_to_b(bipbuf_t& buf_)
{
    if (buf_.size - buf_.a_end < buf_.a_start - buf_.b_end)
        buf_.b_inuse = 1;
}


constexpr unsigned char* offer_begin(bipbuf_t& buf_)
{
    return buf_.data + (1 == buf_.b_inuse ? buf_.b_end : buf_.a_end);
}

// NOTE: Deviates from regular offer in that a manual check for 'size' is required
inline void offer_end(bipbuf_t& buf_, unsigned size)
{
    if (1 == buf_.b_inuse)
    {
        buf_.b_end += size;
    }
    else
    {
        buf_.a_end += size;
    }

    check_for_switch_to_b(buf_);
}

}

namespace layer1 {

template <unsigned N>
class bipbuf
{
    // Although c++ unions are more nuanced than one would presume, it seems safe to
    // expect backing_buffer_ will precisely overlap, based on
    // https://stackoverflow.com/questions/33056403/is-it-legal-to-use-address-of-one-field-of-a-union-to-access-another-field
    // https://stackoverflow.com/questions/11373203/accessing-inactive-union-member-and-undefined-behavior
    // NOTE: Above comment N/A we have left union behind

    byte backing_buffer_[sizeof(bipbuf_t) + N];

    // Can't do union here
    // https://github.com/malachi-iot/estdlib/issues/30

    // DEBT: Our casting gets compilers happier, but still is nonconformant.
    // https://stackoverflow.com/questions/31881811/does-extern-c-with-c-avoid-undefined-behavior-that-is-legal-in-c-but-not-c
    // We have to either ONLY interact with flexible member via C (which we almost do) or go nuclear
    // and re-write it (which I was debating doing anyway)

    bipbuf_t* buf()
    {
        return reinterpret_cast<bipbuf_t*>(backing_buffer_);
    }

    const bipbuf_t* buf() const
    {
        return reinterpret_cast<const bipbuf_t*>(backing_buffer_);
    }

public:
    bipbuf()
    {
        bipbuf_init(buf(), N);
    }

    bipbuf_t* native() { return buf(); }

    unsigned char* offer_begin()
    {
        return internal::offer_begin(*buf());
    }

    /// Low level call, no bounds checking!
    /// @param size
    void offer_end(unsigned size)
    {
        return internal::offer_end(*buf(), size);
    }

    int offer(const unsigned char* d, int size)
    {
        return bipbuf_offer(buf(), d, size);
    }

    // NOTE: Not 100% sure I like auto casting here, not congruent with rest of object
    /*
    template <class T, unsigned N2>
    int offer(const T (&buffer)[N2])
    {
        return bipbuf_offer(&buf_, (const unsigned char*)buffer, N2 * sizeof(T));
    }   */

    const unsigned char* peek(int len = 0) const
    {
        return bipbuf_peek(buf(), len);
    }

    const unsigned char* poll(int len)
    {
        return bipbuf_poll(buf(), len);
    }

    unsigned used() const
    {
        return (unsigned)bipbuf_used(buf());
    }

    unsigned unused() const
    {
        return (unsigned)bipbuf_unused(buf());
    }
};

}

namespace layer3 {

class bipbuf
{
    bipbuf_t* buf_;

public:
    bipbuf(bipbuf_t* buf, int size) : buf_{buf}
    {
        bipbuf_init(buf_, size);
    }

    ESTD_CPP_CONSTEXPR_RET EXPLICIT bipbuf(bipbuf_t* buf) : buf_{buf}
    {
    }

    bipbuf_t* native() { return buf_; }

    unsigned char* offer_begin()
    {
        return internal::offer_begin(*buf_);
    }

    /// Low level call, no bounds checking!
    /// @param size
    void offer_end(unsigned size)
    {
        internal::offer_end(*buf_, size);
    }

    int offer(const unsigned char* d, int size)
    {
        return bipbuf_offer(buf_, d, size);
    }

    const unsigned char* peek(int len = 0) const
    {
        return bipbuf_peek(buf_, len);
    }

    const unsigned char* poll(int len)
    {
        return bipbuf_poll(buf_, len);
    }

    unsigned used() const
    {
        return (unsigned) bipbuf_used(buf_);
    }

    unsigned unused() const
    {
        return (unsigned) bipbuf_unused(buf_);
    }
};

}

}
