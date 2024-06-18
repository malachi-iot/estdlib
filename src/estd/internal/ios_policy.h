#pragma once

#if ESTD_OS_TYPE
#include "../port/thread.h"
#endif
#include "ios_base.h"
#include "stream_flags.h"

namespace estd {


namespace internal {

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
// yield requires presence of an OS
#if ESTD_OS_TYPE
                    estd::this_thread::yield();
#endif
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


template <class Streambuf,
    estd::internal::stream_flags::flag_type flags>
struct ios_base_policy : 
    istream_blocking_policy<Streambuf, flags & estd::internal::istream_flags::block_mask>
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