#pragma once

#include "../port/thread.h"
#include "ios_base.h"

namespace estd {


namespace internal {

// NOTE: May have to make a bunch of duplicate #defines for pre C++0x compilers

// flags applying to both istream and ostream
struct stream_flags
{
    typedef unsigned flag_type;

    // bits 0, 1
    static CONSTEXPR flag_type non_blocking = 0,
        blocking = 1,
        runtime_blocking = 2,
        block_mask = 3,

        // bit 2
        inline_rdbuf = 0,
        traditional_rdbuf = 4,
        rdbuf_mask = 4,

        _end = 4,
        _default = 0;
};

// istream specific flags
struct istream_flags : stream_flags
{
};


template <class TStreambuf, estd::internal::istream_flags::flag_type>
struct istream_blocking_policy;

template <class TStreambuf>
struct istream_blocking_policy<TStreambuf, estd::internal::istream_flags::non_blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename streambuf_type::int_type int_type;

    struct blocking_type
    {
        static int_type sgetc(const ios_base&, streambuf_type* rdbuf)
        {
            return rdbuf->sgetc();
        }

        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz)
        {
            in.setstate(ios_base::nodatabit);
            // EXPERIMENTAL
            // Back off the characters when in nonblocking mode so that one may attempt again
            // DEBT: Consider doing a gbump as an optimization, remembering that it does no
            // underflow checks
            for(unsigned i = sz; i > 0; --i)
                rdbuf->sungetc();
        }
    };
};

template <class TStreambuf>
struct istream_blocking_policy<TStreambuf, estd::internal::istream_flags::blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename streambuf_type::int_type int_type;

    struct blocking_type
    {
        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz) {}

        static int_type sgetc(const ios_base&, streambuf_type* rdbuf)
        {
            for(;;)
            {
                // DEBT: Need timeout logic here
                estd::streamsize avail = rdbuf->in_avail();

                if(avail == -1) return traits_type::eof();

                if(avail == 0)
                {
                    estd::this_thread::yield();
                }
                else
                {
                    return rdbuf->sgetc();
                }
            }
        }
    };
};

template <class TStreambuf>
struct istream_blocking_policy<TStreambuf, estd::internal::istream_flags::runtime_blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::int_type int_type;
    typedef istream_blocking_policy<TStreambuf, estd::internal::istream_flags::non_blocking> nonblocking_policy;
    typedef istream_blocking_policy<TStreambuf, estd::internal::istream_flags::blocking> blocking_policy;

    // FIX: Need to interrogate 'in' to determine whether we're blocking or not blocking
    struct blocking_type
    {
        static void on_nodata(ios_base& in, streambuf_type* rdbuf, unsigned sz)
        {
            blocking_policy::on_nodata(in, rdbuf, sz);
        }

        static int_type sgetc(const ios_base& in, streambuf_type* rdbuf)
        {
            return blocking_policy::sgetc(in, rdbuf);
        }
    };
};


template <class TStreambuf,
    estd::internal::stream_flags::flag_type flags = estd::internal::stream_flags::_default>
struct ios_base_policy : 
    istream_blocking_policy<TStreambuf, flags & estd::internal::istream_flags::block_mask>
{
    // DEBT: Clearly we don't want this unconfigurable and always set to en_US.UTF-8
    typedef locale<internal::locale_code::en_US, encodings::UTF8>
        locale_type;

    static CONSTEXPR estd::internal::stream_flags::flag_type blocking()
    {
        return flags & estd::internal::stream_flags::block_mask;
    }
};

}

}