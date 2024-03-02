#pragma once

extern "C" {
#include "../../ext/willemt/bipbuffer/bipbuffer.h"
}

namespace estd { namespace internal {

namespace layer1 {

// DEBT: Move these to a cpp or hpp somewhere
// more or less copy/pasting & splitting out what bipbuf_offer does

inline void check_for_switch_to_b(bipbuf_t& buf_)
{
    if (buf_.size - buf_.a_end < buf_.a_start - buf_.b_end)
        buf_.b_inuse = 1;
}


inline unsigned char* offer_begin(bipbuf_t& buf_)
{
    return buf_.data + (1 == buf_.b_inuse ? buf_.b_end : buf_.a_end);
}

inline int offer_end(bipbuf_t& buf_, int size)
{
    if (bipbuf_unused(&buf_) < size)
        return 0;

    if (1 == buf_.b_inuse)
    {
        buf_.b_end += size;
    }
    else
    {
        buf_.a_end += size;
    }

    check_for_switch_to_b(buf_);

    return size;
}


template <unsigned N>
class bipbuf
{
    // DEBT: I read that we're not guaranteed for backing_buffer_ to start at same spot as
    // buf_.  I believe the usage we have here is 100% safe though.  Needs a double check
    union
    {
        byte backing_buffer_[sizeof(bipbuf_t) + N];
        bipbuf_t buf_;
    };

public:
    bipbuf()
    {
        bipbuf_init(&buf_, N);
    }

    bipbuf_t* native() { return &buf_; }

    unsigned char* offer_begin()
    {
        return layer1::offer_begin(buf_);
    }

    int offer_end(int size)
    {
        return layer1::offer_end(buf_, size);
    }

    int offer(const unsigned char* d, int size)
    {
        return bipbuf_offer(&buf_, d, size);
    }

    unsigned char* peek(int len = 0)
    {
        return bipbuf_peek(&buf_, len);
    }

    const unsigned char* peek(int len = 0) const
    {
        return bipbuf_peek(&buf_, len);
    }

    unsigned char* poll(int len)
    {
        return bipbuf_poll(&buf_, len);
    }

    int used() const
    {
        return bipbuf_used(&buf_);
    }

    int unused() const
    {
        return bipbuf_unused(&buf_);
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

    bipbuf(bipbuf_t* buf) : buf_{buf}
    {
    }

    bipbuf_t* native() { return buf_; }

    unsigned char* offer_begin()
    {
        return layer1::offer_begin(*buf_);
    }

    int offer_end(int size)
    {
        return layer1::offer_end(*buf_, size);
    }

    int offer(const unsigned char* d, int size)
    {
        return bipbuf_offer(buf_, d, size);
    }

    unsigned char* peek(int len = 0)
    {
        return bipbuf_peek(buf_, len);
    }

    const unsigned char* peek(int len = 0) const
    {
        return bipbuf_peek(buf_, len);
    }

    unsigned char* poll(int len)
    {
        return bipbuf_poll(buf_, len);
    }

    int used() const
    {
        return bipbuf_used(buf_);
    }

    int unused() const
    {
        return bipbuf_unused(buf_);
    }
};

}

}}
