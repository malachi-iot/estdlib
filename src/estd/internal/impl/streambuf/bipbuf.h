#pragma once

#include "base.h"
#include "../../bip/buffer.h"

namespace estd { namespace internal { namespace impl {

// DEBT: This is a layer1-specific flavor which is OK and useful, but a layer2 variety
// would be good as well.  The way willemt's implementation works that would amount to
// a pure bipbuf_t pointer. Maybe less relevant now that this is an alias and not
// a member type
// 21MAY24 DEBT: Poor naming
template <unsigned len>
using adapt_bipbuf = estd::conditional_t<(len > 0),
        estd::layer1::bipbuf<len>,
        estd::layer3::bipbuf>;


// 21MAY24 TODO: Consider representing this as our own 'syncbuf' as per
// https://en.cppreference.com/w/cpp/io/basic_syncbuf.  I just discovered
// this class today

// TODO: Do also a flavor of this using esp-idf's xRingbuffer which itself
// seems to be an RTOS-friendly bipbuffer
template <ESTD_CPP_CONCEPT(concepts::v1::OutStreambuf) Streambuf, unsigned len>
class out_buffered_bipbuf : public wrapped_streambuf_base<Streambuf>
{
    using base_type = wrapped_streambuf_base<Streambuf>;

public:
    using typename base_type::int_type;
    using typename base_type::char_type;
    using typename base_type::traits_type;


private:

    // https://github.com/willemt/bipbuffer
    // abusing mutable to conform with constness of pptr()
    mutable adapt_bipbuf<sizeof(char_type) * len> buf_;

protected:
    unsigned xout_avail() const
    {
        return buf_.unused() / sizeof(char_type);
    }

    void pbump(int count)
    {
        buf_.offer_end(count);
    }

    int sync()
    {
        // DEBT: A lot of tuning opportunity here
        const unsigned sz = buf_.used();
        auto b = (char_type*)buf_.peek(sz);
        unsigned written = base_type::rdbuf().sputn(b, sz);
        if(written == 0) return -1;
        buf_.poll(written);
        return 0;
    }

public:
    //char_type* pbase() const { return buf_.data(); }
    char_type* pptr() const { return reinterpret_cast<char_type*>(buf_.offer_begin()); }
    char_type* epptr() const { return pptr() + xout_avail(); }

    int_type sputc(char_type ch)
    {
        return overflow(ch);
    }

protected:
    template <class ...Args>
#if __cpp_concepts
        // Crude attempt to verify protected signatures.  Works,
        // but only when placed in this spot near templated args.  Placed
        // below function signature inhibits compilation without any clue as
        // to the concept failure reason - useless
        //requires concepts::v1::impl::OutStreambuf<out_buffered_bipbuf>
#endif
    // 21MAY24 DEBT: Probably we want in_place_t here to disambiguate edge
    // case where base_type itself wants a bipbuf_t*
    explicit out_buffered_bipbuf(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {
    }

    template <class ...Args>
    explicit out_buffered_bipbuf(bipbuf_t* buf, Args&&...args) :
        base_type(std::forward<Args>(args)...),
        buf_(buf)
    {
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
        if(count > (int)out_avail)
            count = out_avail;

        // NOTE: Guaranteed to work since we trim to available up above, no need
        // to check result code
        buf_.offer((const unsigned char*)s, count * sizeof(char_type));

#if FEATURE_ESTD_STREAMBUF_TRAITS
        // set_dtr somehow, but I'm not convinced yet how homogeneous this will be.
        // (consider netconn callback SEND+ behavior)
#endif

        return count;
    }
};


// NOTE: Be advised,
// https://en.cppreference.com/w/cpp/io/basic_streambuf/pubsync indicates
// "For input streams, this typically empties the get area and forces a re-read from the associated sequence to pick up recent changes"
// which is almost the opposite of what we do here.

template <class Streambuf, unsigned len>
class in_buffered_bipbuf : public wrapped_streambuf_base<Streambuf>
{
    using base_type = wrapped_streambuf_base<Streambuf>;

public:
    using typename base_type::int_type;
    using typename base_type::char_type;
    using typename base_type::traits_type;
    using nonconst_char_type = remove_const_t<char_type>;

private:
    adapt_bipbuf<sizeof(char_type) * len> buf_;

protected:
    // 21MAY24 DEBT: It's unclear what constitutes a failure here (-1).
    // I infer that ONLY permanent EOF denotes this:
    // https://en.cppreference.com/w/cpp/io/basic_streambuf/pubsync
    // If so, these -1's in our implementation are overstated
    int sync()
    {
        const unsigned unused = buf_.unused();

        if(unused == 0)    return -1;

        auto buffer = (char_type*) buf_.offer_begin();

        const streamsize actually_read = base_type::rdbuf().sgetn(buffer, unused);

        if(actually_read == 0)  return -1;

        buf_.offer_end(actually_read);

        return 0;
    }

    constexpr streamsize xin_avail() const
    {
        return buf_.used();
    }

    // DEBT: This needs to interrogate wrapped Streambuf
    constexpr streamsize showmanyc() const
    {
        return xin_avail();
    }

    streamsize xsgetn(nonconst_char_type* s, streamsize count)
    {
        if(count > xin_avail())
            count = xin_avail();

        copy_n(buf_.peek(), count, s);

        return count;
    }

    ESTD_CPP_FORWARDING_CTOR(in_buffered_bipbuf)

    int_type underflow()
    {
        if(xin_avail() == 0)
        {
            // DEBT: Return proper eof
            if(sync() == -1)    return -1;
        }

        // DEBT: Wrap with traits
        return xsgetc();
    }

    int_type uflow()
    {
        // DEBT: ADL + data hiding may be an issue here
        //return estd::internal::impl::uflow(this);

        const int_type c = underflow();

        if(c != traits_type::eof())
            gbump(1);

        return c;
    }

    void gbump(int count)
    {
        buf_.poll(count);
    }

    char_type xsgetc() const { return *gptr(); }

public:
    // DEBT: Deviation from spec, which doesn't specify const pointer here
    const char_type* gptr() const
    {
        return reinterpret_cast<const char_type*>(buf_.peek());
    }

    const char_type* egptr() const
    {
        return gptr() + buf_.used();
    }
};

}}}
