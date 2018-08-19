#pragma once

#include "../../traits/char_traits.h"
#include "../ios.h"
#include "../../queue.h" // for out_queue_streambuf

namespace estd { namespace internal { namespace impl {

// NOTE: Temporarily making a 'complete' type, but plan is to make this always specialized
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf;

// Expect most native stream implementations can use this helper base impl
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf_base
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



template <class TString>
struct out_stringbuf
{
    typedef typename remove_reference<TString>::type string_type;
    typedef typename string_type::value_type char_type;
    typedef typename string_type::traits_type traits_type;

    TString _str;

    out_stringbuf() {}

    template <class TParam1>
    out_stringbuf(TParam1& p) : _str(p) {}

    streamsize xsputn(const char_type* s, streamsize count)
    {
        // FIX: normal strings throw an exception if we exceed internal
        // buffer size, but here we should instead have an optional error
        // facility
        _str.append(s, count);
        return count;
    }

    // deviates from spec in that this is NOT a copy, but rather a direct reference
    // to the tracked string.  Take care
    string_type& str() { return _str; }
};


template <class TString>
struct basic_stringbuf : out_stringbuf<TString>
{
    typedef out_stringbuf<TString> base_type;
    typedef typename base_type::traits_type traits_type;
    typedef typename base_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_type::string_type string_type;
    typedef typename string_type::size_type size_type;

    size_type get_pos;

    basic_stringbuf() : get_pos(0) {}

    template <class TParam1>
    basic_stringbuf(TParam1& p) :
        base_type(p),
        get_pos(0) {}

    streamsize xsgetn(char_type* s, streamsize count)
    {
        streamsize orig_count = count;
        const char_type* src = base_type::_str.clock(get_pos, count);

        while(count--) *s++ = *src++;

        base_type::_str.unlock();
        get_pos += orig_count;
        return orig_count;
    }

    int_type sgetc()
    {
        // no 'underflow' for a basic string.  no more chars means no more chars, plain
        // and simple
        if(get_pos == base_type::_str.length())
            return traits_type::eof();

        const char_type ch = *base_type::_str.clock(get_pos, 1);
        base_type::_str.cunlock();
        return ch;
    }

    // NOTE: This leaves things unlocked, so only enable this for layer1-layer3 strings
    // this implicitly is the case as we do not implement 'data()' except for scenarios
    // where locking/unlocking is a noop (or otherwise inconsequential)
    char_type* gptr() { return base_type::_str.data() + get_pos; }
};

// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits = ::std::char_traits<TChar> >
struct basic_streambuf
{
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;
    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
    virtual int sync() = 0;
};

}}}
