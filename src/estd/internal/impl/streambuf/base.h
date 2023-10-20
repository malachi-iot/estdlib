#pragma once

#include "../../ios.h"

namespace estd { namespace internal { namespace impl {

/// @brief contains base noop-ish implementation, suitable for hiding (think override,
/// but without the virtual since we're all templated)
/// Derived classes are expected to implement:
/// - sgetc
/// - sputc
/// - xsgetn
/// - xsputn
/// - gbump
template <class TTraits>
struct streambuf_base
{
    typedef TTraits traits_type;
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


}}}