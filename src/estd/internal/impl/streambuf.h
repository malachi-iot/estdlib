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

template <typename TPos>
struct pos_streambuf_base
{
    typedef TPos pos_type;

    pos_type _pos;

    pos_streambuf_base(pos_type pos) : _pos(pos) {}

    pos_type pos() const { return _pos; }

    // FIX: Need this because ESTD_FN_HAS_METHOD falls on its face for detecting overloaded methods
    pos_type get_pos() const { return pos(); }

protected:
    void pos(pos_type p) { _pos = p; }
};

template <typename TCharTraits>
struct in_pos_streambuf_base : pos_streambuf_base<typename TCharTraits::pos_type>
{
    typedef pos_streambuf_base<typename TCharTraits::pos_type> base_type;
    typedef typename base_type::pos_type pos_type;

    in_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void gbump(int count) { this->_pos += count; }
};


template <typename TCharTraits>
struct out_pos_streambuf_base : pos_streambuf_base<typename TCharTraits::pos_type>
{
    typedef pos_streambuf_base<typename TCharTraits::pos_type> base_type;
    typedef typename base_type::pos_type pos_type;

    out_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void pbump(int count) { this->_pos += count; }
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


// TODO: decouple all these stringbufs so we can have a standalone in_stringbuf
template <class TString>
struct stringbuf_base
{
    stringbuf_base() {}
};

// TODO: utilize out_pos_streambuf/in_pos_streambuf
template <class TString>
struct out_stringbuf : stringbuf_base<TString>
{
    typedef typename remove_reference<TString>::type string_type;
    typedef typename string_type::value_type char_type;
    typedef typename string_type::traits_type traits_type;
    typedef typename traits_type::off_type off_type;
    typedef typename traits_type::pos_type pos_type;

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
    const string_type& str() const { return _str; }

    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which)
    {
        // TODO: check to make sure which contains ios_base::out
        return _str.size();
    }
};


template <class TString>
struct in_stringbuf
{

};

template <class TString>
struct basic_stringbuf :
        out_stringbuf<TString>,
        in_stringbuf<TString>
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

    streamsize showmanyc() const
    {
        size_type len = this->_str.length() - get_pos;
        return len > 0 ? len : -1;
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

    void gbump(int n) { get_pos += n; }
};



// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
// TODO: Do char_traits the same between this and in_span_streambuf
template <class T, std::ptrdiff_t Extent = -1,
          class TBase = experimental::instance_provider<estd::span<T, Extent> > >
struct out_span_streambuf :
        out_pos_streambuf_base<estd::char_traits<T> >,
        TBase
{
    typedef TBase base_type;
    typedef T char_type;
    typedef estd::char_traits<char_type> traits_type;
    typedef out_pos_streambuf_base<estd::char_traits<T> > base_out_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef typename traits_type::off_type off_type;
    typedef typename traits_type::pos_type pos_type;

    span_type& out() { return base_type::value(); }
    const span_type& out() const { return base_type::value(); }

    out_span_streambuf(T* buf, size_type size) :
        base_type(span_type(buf, size))
    {

    }

    // NOTE: Would use Extent here but that breaks it for scenarios
    // where Extent == -1
    template <std::size_t N>
    out_span_streambuf(T (&array)[N]) :
        base_type(array)
    {

    }

    out_span_streambuf(const estd::span<T, Extent>& copy_from) :
        base_type(copy_from)
    {

    }


    out_span_streambuf(char_type* data, pos_type count) :
            base_type(data, count)
    {

    }

    char_type* pbase() const
    {
        return const_cast<char_type*>(out().data());
    }

    char_type* pptr() const { return pbase() + base_out_type::pos(); }
    char_type* epptr() const { return pbase() + out().size_bytes(); }

    streamsize xsputn(const char_type* s, streamsize count)
    {
        // TODO: do proper bounds checking here
        memcpy(pptr(), s, count);
        this->pbump(count);
        return count;
    }

protected:
    // NOTE: currently undefined if pos ends up outside boundaries
    // NOTE: consider changing dir and which to template non-type parameters/compile
    // time constants
    pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode which)
    {
        // openmode MUST include 'out' in this instance, otherwise error or ignore
        if(!(which & ios_base::out)) return -1;

        switch(way)
        {
            case ios_base::cur:
                this->pbump(off);
                break;

            case ios_base::beg:
                base_out_type::pos(off);
                break;

            case ios_base::end:
                // UNTESTED
                base_out_type::pos(out().size_bytes() + off);
                break;
        }

        return base_out_type::pos();
    }

public:
    void setbuf_experimental(char_type* s, streamsize n)
    {
        base_out_type::pos(0);
        base_type::value(span<char_type>(s, n));
    }
};


// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
template <class TChar,
        class TCharTraits =  estd::char_traits<TChar>,
        std::ptrdiff_t Extent = -1,
        class TBase = experimental::instance_provider<estd::span<TChar, Extent> > >
struct in_span_streambuf :
        in_pos_streambuf_base<TCharTraits>,
        TBase
{
    typedef in_pos_streambuf_base<TCharTraits> base_pos_type;
    typedef TBase base_type;

    typedef TCharTraits traits_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_pos_type::pos_type pos_type;
    typedef TChar char_type;
    typedef typename remove_const<char_type>::type nonconst_char_type;

    const span_type& in() const { return base_type::value(); }

    pos_type pos() const { return base_pos_type::pos(); }
    void pos(pos_type p) { base_pos_type::pos(p); }

    in_span_streambuf(const estd::span<TChar, Extent>& copy_from) :
        base_type(copy_from)
    {

    }

    // FIX: dropping const on returned span_type, not recommended
    char_type* eback() const
    { return const_cast<char_type*>(in().data()); }

    char_type* gptr() const { return eback() + pos(); }
    char_type* egptr() const { return eback() + in().size(); }

private:
    streamsize remaining() const { return in().size() - pos(); }

protected:
    // EXPERIMENTAL - counts on the idea that underflow would *only*
    // be called when gptr() == egptr().  This may or may not really
    // be correct
    int_type underflow() { return traits_type::eof(); }

    streamsize showmanyc() const
    {
        streamsize r = remaining();

        // there is never a time we are unsure if more characters remain in this type of span buffer
        return r > 0 ? r : -1;
    }

    streamsize xsgetn(nonconst_char_type* s, streamsize count)
    {
        // NOTE: No uflow/eof handling since a span unlike a netbuf is just one buffer and that's it
        streamsize c = estd::min(count, remaining());
        estd::copy_n(gptr(), c, s);
        this->gbump(c);
        return c;
    }

public:
    // NOTE: This would preferably not be in impl part
    int_type sgetc()
    {
        // NOTE: non-span versions would call onto underflow here
        // however, for a span, underflow never will fetch a new buffer so
        // we don't do it
        if(remaining() == 0)
            //return traits_type::eof();
            return underflow(); // EXPERIMENTAL - always returns eof() for this class

        int_type ch = traits_type::to_int_type(*gptr());

        return ch;
    }
};


// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits = estd::char_traits<TChar> >
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
