#pragma once

#include "ios_base.h"

namespace estd {


namespace internal {

// NOTE: May have to make a bunch of duplicate #defines for pre C++0x compilers

struct istream_flags
{
    typedef unsigned flag_type;

    // bits 0, 1
    static constexpr flag_type non_blocking = 0,
        blocking = 1,
        runtime_blocking = 2,
        block_mask = 3,

    // bit 2
        inline_rdbuf = 0,
        traditional_rdbuf = 4,
        rdbuf_mask = 4,

        _default = 0;
};


template <class TStreambuf, estd::internal::istream_flags::flag_type>
struct ios_blocking_policy;

template <class TStreambuf>
struct ios_blocking_policy<TStreambuf, estd::internal::istream_flags::non_blocking>
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
struct ios_blocking_policy<TStreambuf, estd::internal::istream_flags::blocking>
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
struct ios_blocking_policy<TStreambuf, estd::internal::istream_flags::runtime_blocking>
{
    typedef typename estd::remove_reference<TStreambuf>::type streambuf_type;
    typedef typename streambuf_type::int_type int_type;
    typedef ios_blocking_policy<TStreambuf, estd::internal::istream_flags::non_blocking> nonblocking_policy;
    typedef ios_blocking_policy<TStreambuf, estd::internal::istream_flags::blocking> blocking_policy;

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
    estd::internal::istream_flags::flag_type flags =
        estd::internal::istream_flags::blocking> // FIX: Temporarily set to blocking as we code out feature
        //estd::experimental::istream_flags::_default>
struct ios_base_policy : 
    ios_blocking_policy<TStreambuf, flags & estd::internal::istream_flags::block_mask>
{
    typedef experimental::locale locale_type;

    static CONSTEXPR estd::internal::istream_flags::flag_type blocking()
    {
        return flags & estd::internal::istream_flags::block_mask;
    }
};

}

}