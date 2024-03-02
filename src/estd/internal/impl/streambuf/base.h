#pragma once

#include "fwd.h"

namespace estd { namespace internal { namespace impl {

/// @brief contains base noop-ish implementation, suitable for hiding (think override,
/// but without the virtual since we're all templated)
/// Derived classes are expected to implement:
/// - sgetc
/// - sputc
/// - xsgetn
/// - xsputn
/// - gbump
template <ESTD_CPP_CONCEPT(concepts::v1::CharTraits) Traits>
struct streambuf_base
{
    typedef Traits traits_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

protected:
    inline static int sync() { return 0; }

    // 0 = "unsure if there are characters available in the associated sequence".
    inline static streamsize showmanyc() { return 0; }

    inline static pos_type seekpos(pos_type, ios_base::openmode)
    {
        return pos_type(off_type(-1));
    };

    inline static pos_type seekoff(off_type, ios_base::seekdir, ios_base::openmode)
    {
        return pos_type(off_type(-1));
    };

    inline static int_type uflow() { return traits_type::eof(); }

    inline static int_type pbackfail(int_type c = traits_type::eof())
    {
        return traits_type::eof();
    }

    inline static int_type overflow(int_type ch = traits_type::eof())
    {
        return traits_type::eof();
    }

    inline static int_type underflow() { return traits_type::eof(); }

    // Non-standard API feeder for showmanyc/in_avail
    // Only reports on available or not, does not take any guesses
    // as to what might come if the buffer is filled
    inline static int_type xin_avail() { return 0; }

    // Helper to produce showmanyc-style return values from regular
    // in avail style values.  Remember "0" for showmanyc means
    // unknown character availabity
    inline static streamsize showmanyc(int_type avail, bool eof = true)
    {
        if(eof)
            // eof flag tells us whether there's any ambiguity about actually being end of buffer
            return avail > 0 ? avail : -1;
        else
            return avail;
    }
};


// Expect most native stream implementations can use this helper base impl
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf_base : streambuf_base<TCharTraits>
{
    // NOTE: we'll need to revisit this if we want a proper pointer in here
    typedef typename estd::remove_reference<TStream>::type stream_type;
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    TStream stream;

    native_streambuf_base(stream_type& stream) : stream(stream)
    {}

#if __cpp_rvalue_reference
    native_streambuf_base(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif
};


// For streambufs which wrap other streambufs, this is a convenient helper
// DEBT: Really should be worked out with instance_provider/instance_evaporator
// and friends
template <class Streambuf>
class wrapped_streambuf_base :
    public streambuf_base<typename remove_reference<Streambuf>::type::traits_type>
{
protected:
    typedef typename remove_reference<Streambuf>::type streambuf_type;

    // TODO: Make this an is-a not a has-a so we can do both istreambuf and ostreambuf
    Streambuf streambuf_;

    ESTD_CPP_FORWARDING_CTOR_MEMBER(wrapped_streambuf_base, streambuf_)

public:
    // Goofy but sensible rdbuf() resulting in possible rdbuf()->rdbuf() calls
    streambuf_type& rdbuf() { return streambuf_; }
    const streambuf_type& rdbuf() const { return streambuf_; }
};

}}}