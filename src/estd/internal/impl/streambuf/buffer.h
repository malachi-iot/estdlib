#pragma once

#include "base.h"
#include "../../fwd/string.h"

extern "C" {
#include "../../../ext/willemt/bipbuffer/bipbuffer.h"
}

// Ironic, we have to put the true-blue pseudo-double-buffer that streambuf represents into a distinct
// 'buffer' area

namespace estd { namespace internal { namespace impl {

#if __cplusplus >= 201103L
// This class has nothing specific inhibiting c++03 compat, I am just saving time.
// Hard wired to layer1 string. a span version of this would be nice too
// DEBT: It's likely we can combine this with existing span buf and stringbuf to have a noop-Streambuf
// in them
template <class Streambuf, unsigned len = 64>
class out_buffered_stringbuf : public wrapped_streambuf_base<Streambuf>
{
    using base_type = wrapped_streambuf_base<Streambuf>;

    static constexpr int len_ = len;

    // TODO: Make this a string type we pass in somehow
    estd::layer1::string<len_, false> buf_;

    //constexpr
    unsigned remaining() const
    {
        unsigned m = buf_.max_size();
        unsigned s = buf_.size();
        //return buf_.max_size() - buf_.size();
        return m - s;
    }

    // DEBT: Finish up string/allocated_array underlying full implementation
    constexpr bool full() const
    {
        return remaining() == 0;
    }

    int ll_sync()
    {
        // NOTE: This class only syncs via bulk write
        // DEBT: We can expect potential spinlock type behavior,
        // which may be needed due to the simplicity of string buffering
        // (may need a queue or bipbuffer to really overcome that)
        base_type::rdbuf().sputn(buf_.data(), buf_.size());
        buf_.clear();
        // DEBT: Need to heed output of sputn to truly indicate status
        return 0;
    }

public:
    using typename base_type::int_type;
    using typename base_type::char_type;
    using typename base_type::traits_type;

protected:
    int sync()
    {
        if(buf_.empty()) return 0;

        return ll_sync();
    }

    int_type overflow(int_type ch)
    {
        if(full())
        {
            if(ll_sync() != 0)  return traits_type::eof();
        }

        if(traits_type::not_eof(ch))
        {
            buf_ += traits_type::to_char_type(ch);
        }

        // "Returns unspecified value not equal to Traits::eof() on success"
        // DEBT: In some universe, '*' might be EOF, so watch out here
        return '*';
    }

    estd::streamsize xsputn(const char_type* s, estd::streamsize count)
    {
        const estd::streamsize maximum = remaining();

        buf_.append(s, count > maximum ? maximum : count);

        // wrapper sputn handles this via 'overflow' - a little clumsy though
        //if(full())
        //    ll_sync();

        return count;
    }

public:
    ESTD_CPP_FORWARDING_CTOR(out_buffered_stringbuf)

    char_type* pbase() const { return buf_.data(); }
    char_type* pptr() const { return pbase() + buf_.size(); }
    char_type* epptr() const { return pbase() + buf_.max_size(); }

    int_type sputc(char_type ch)
    {
        return overflow(ch);
    }
};


// TODO: Do also a flavor of this using esp-idf's xRingbuffer which itself
// seems to be an RTOS-friendly bipbuffer
template <class Streambuf, unsigned len = 64>
class out_buffered_bipbuf : public wrapped_streambuf_base<Streambuf>
{
    using base_type = wrapped_streambuf_base<Streambuf>;

    // DEBT: This is a layer1-specific flavor which is OK and useful, but a layer3 variety
    // would be good as well.  The way willemt's implementation works that would amount to
    // a pure bipbuf_t pointer.

    // https://github.com/willemt/bipbuffer
    //bipbuf_t* buf_;

    // DEBT: I read that we're not guaranteed for backing_buffer_ to start at same spot as
    // buf_.  I believe the usage we have here is 100% safe though.  Needs a double check
    union
    {
        byte backing_buffer_[sizeof(bipbuf_t) + len];
        // abusing mutable to conform with constness of pptr()
        mutable bipbuf_t buf_;
    };

    // more or less copy/pasting & splitting out what bipbuf_offer does

    void check_for_switch_to_b()
    {
        if (buf_.size - buf_.a_end < buf_.a_start - buf_.b_end)
            buf_.b_inuse = 1;
    }


    unsigned char* offer_begin() const
    {
        return buf_.data + (1 == buf_.b_inuse ? buf_.b_end : buf_.a_end);
    }

    int offer_end(int size)
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

        check_for_switch_to_b();

        return size;
    }

protected:
    unsigned xout_avail() const
    {
        return bipbuf_unused(&buf_) / sizeof(char_type);
    }

    void pbump(int count)
    {
        offer_end(count);
    }

    int sync()
    {
        unsigned sz = bipbuf_used(&buf_);
        auto b = (char_type*)bipbuf_poll(&buf_, sz);
        base_type::rdbuf().sputn(b, sz);
        return 0;
    }

public:
    using typename base_type::int_type;
    using typename base_type::char_type;
    using typename base_type::traits_type;

    //char_type* pbase() const { return buf_.data(); }
    char_type* pptr() const { return reinterpret_cast<char_type*>(offer_begin()); }
    char_type* epptr() const { return pptr() + xout_avail(); }

    int_type sputc(char_type ch)
    {
        return overflow(ch);
    }

protected:
    // DEBT: c++ doesn't like init this way PLUS we need a forwarding constructor here
    out_buffered_bipbuf()
    {
        bipbuf_init(&buf_, len);
    }

    int_type overflow(int_type ch)
    {
        if(xout_avail() == 0)
            sync();

        *pptr() = traits_type::to_char_type(ch);
        pbump(1);

        return ch;
    }

    estd::streamsize xsputn(const char_type* s, estd::streamsize count)
    {
        const unsigned out_avail = xout_avail();
        if(count > out_avail)
            count = out_avail;

        // NOTE: Guaranteed to work since we trim to available up above, no need
        // to check result code
        bipbuf_offer(&buf_, (const unsigned char*)s, count * sizeof(char_type));

        return count;
    }
};



#endif

}}}