#pragma once

#include "../../traits/char_traits.h"
#include "../ios.h"
#include "../../queue.h" // for out_queue_streambuf
#include "../../span.h" // for span streambuf

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


// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
template <class T, std::ptrdiff_t Extent = -1,
          class TBase = experimental::instance_provider<estd::span<T, Extent> > >
struct out_span_streambuf : TBase
{
    typedef TBase base_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef int off_type;
    typedef int pos_type;

    span_type& out() { return base_type::value(); }
    const span_type& out() const { return base_type::value(); }

    size_type pos;

    typedef char char_type;
    typedef std::char_traits<char_type> traits_type;

    out_span_streambuf(T* buf, size_type size) :
        base_type(span_type(buf, size)),
        pos(0)
    {

    }

    // NOTE: Would use Extent here but that breaks it for scenarios
    // where Extent == -1
    template <std::size_t N>
    out_span_streambuf(T (&array)[N]) :
        base_type(array),
        pos(0)
    {

    }

    out_span_streambuf(const estd::span<T, Extent>& copy_from) :
        base_type(copy_from),
        pos(0)
    {

    }

    char_type* pbase() const
    {
        // FIX: naughty, dropping const here, because underlying instance
        // provider is set to track instance not pointer
        T* data = (T*)out().data();
        return reinterpret_cast<char_type*>(data);
    }
    char_type* pptr() const { return pbase() + pos; }
    char_type* epptr() const { return pbase() + out().size_bytes(); }

    streamsize xsputn(const char_type* s, streamsize count)
    {
        // TODO: do proper bounds checking here
        memcpy(pptr(), s, count);
        pos += count;
        return count;
    }

protected:
    // NOTE: currently undefined if pos ends up outside boundaries
    // NOTE: consider changing dir and which to template non-type parameters/compile
    // time constants
    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which)
    {
        // openmode MUST include 'out' in this instance, otherwise error or ignore
        switch(dir)
        {
            case ios_base::cur:
                pos += off;
                break;

            case ios_base::beg:
                pos = off;
                break;
        }

        return pos;
    }

    // NOTE: could be useful in constructing an automatic pubseekoff, if an explicit one
    // isn't available
    void pbump(int count)
    {
        pos += count;
    }
};


// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
template <class T, std::ptrdiff_t Extent = -1,
        class TBase = experimental::instance_provider<estd::span<T, Extent> > >
struct in_span_streambuf : TBase
{
    typedef TBase base_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;

    span_type& in() { return base_type::value(); }

    size_t pos;

    typedef char char_type;

    in_span_streambuf(const estd::span<T, Extent>& copy_from) :
        base_type(copy_from),
        pos(0)
    {

    }

    char_type* eback() const { return static_cast<char_type*>(in().data()); }
    char_type* gptr() const { return eback() + pos; }
    char_type* egptr() const { return eback() + in().size_bytes(); }
};


// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits = ::std::char_traits<TChar> >
struct basic_streambuf
{
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    //virtual int_type overflow(int_type ch) = 0;
    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;
    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
    virtual int sync() = 0;
};

}}}
