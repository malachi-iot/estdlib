#pragma once

#include "../../traits/char_traits.h"
#include "../ios.h"
#include "../../queue.h" // for out_queue_streambuf
#include "../../span.h" // for span streambuf

namespace estd { namespace internal { namespace impl {

template <class TChar, class TCharTraits = estd::char_traits<TChar> >
struct basic_streambuf;

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
#if !defined(FEATURE_STD_STRING_FULL_CHAR_TRAITS)
    typedef int_type pos_type;
    typedef int_type off_type;
#else
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;
#endif

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

    // Non-standard helper for showmanyc/in_avail
    // Only reports on available or not, does not take any guesses
    // as to what might come if the buffer is filled
    inline static int_type xin_avail() { return 0; }
};


// NOTE: Temporarily making a 'complete' type, but plan is to make this always specialized
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf;

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

#ifdef FEATURE_CPP_MOVESEMANTIC
    native_streambuf_base(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif
};


template <class T, size_t N>
struct layer1_queue_policy
{
    typedef estd::layer1::deque<T, N> queue_type;
};



// EXPERIMENTAL
// but I don't expect API to change once it's settled, so not marking as experimental
template <class TWrappedStreambuf, class TPolicy = layer1_queue_policy<char, 64> >
struct out_queue_streambuf
{
    typedef typename TWrappedStreambuf::char_type char_type;
private:
    TWrappedStreambuf _rdbuf;

    typename TPolicy::queue_type put_area;

public:
    streamsize xsputn(const char_type* s, streamsize count)
    {
        streamsize orig_count = count;

        while(count--)
        {
            put_area.push_back(*s++);
        }

        return orig_count;
    }

    int sync()
    {
        // TODO: Keep bumping into needing intrusive inspectors into queue to
        // get at underlying data buffers
        // _rdbuf.xsputn(put_area.data1());
        return 0;
    }
};





// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits>
struct basic_streambuf
{
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    virtual int_type overflow(int_type ch) = 0;

    // DEBT: Needs to be abstract virtual here
    virtual int_type underflow() { return traits_type::eof(); }


    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;
    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
    virtual int sync() = 0;
};

}}}

#include "streambuf/span.h"
