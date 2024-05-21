#pragma once

#include "../ios.h"
#include "streambuf/base.h"
#include "../../queue.h" // for out_queue_streambuf
#include "../../span.h" // for span streambuf
#include "streambuf/bipbuf.h"
#include "streambuf/buffer.h"

namespace estd { namespace internal { namespace impl {


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
    // DEBT: A little clumsy.  See FEATURE_ESTD_STREAMBUF_TRAITS
    using traits_type = typename streambuf_base<TCharTraits>::traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    virtual int_type overflow(int_type ch) = 0;

    // DEBT: Needs to be abstract virtual here
    virtual int_type underflow() { return traits_type::eof(); }

    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;
    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
    virtual int sync() = 0;

public:
    // DEBT: Not well thought out, just getting things compiling
    int_type sputc(char_type ch) { return overflow(ch); }
};

}}}

#include "streambuf/span.h"
