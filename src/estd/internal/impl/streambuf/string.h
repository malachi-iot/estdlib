#pragma once

#include "../streambuf.h"

namespace estd { namespace internal { namespace impl {

// TODO: decouple all these stringbufs so we can have a standalone in_stringbuf
template <class TString>
struct stringbuf_base : streambuf_base<typename TString::traits_type>
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
    typedef typename traits_type::int_type int_type;

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

    int_type sputc(char_type ch)
    {
        _str += ch;
        return traits_type::to_int_type(ch);
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
struct in_stringbuf : in_pos_streambuf_base<typename TString::traits_type>
{
    typedef in_pos_streambuf_base<typename TString::traits_type> base_type;

    typedef typename base_type::traits_type traits_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;
};

template <class TString>
struct basic_stringbuf :
        out_stringbuf<TString>,
        in_stringbuf<TString>
{
    typedef out_stringbuf<TString> base_type;
    typedef in_stringbuf<TString> in_base_type;
    typedef typename base_type::traits_type traits_type;
    typedef typename base_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_type::string_type string_type;
    typedef typename string_type::size_type size_type;

    basic_stringbuf() {}

    template <class TParam1>
    basic_stringbuf(TParam1& p) :
            base_type(p)
    {}

    streamsize xsgetn(char_type* s, streamsize count)
    {
        size_type count_copied = base_type::str().copy(s, count, in_base_type::pos());

        in_base_type::gbump(count_copied);

        return count_copied;
    }

    size_type remaining() const
    {
        return this->_str.length() - in_base_type::pos();
    }

    streamsize showmanyc() const
    {
        size_type len = remaining();
        return len > 0 ? len : -1;
    }

    int_type sgetc()
    {
        // no 'underflow' for a basic string.  no more chars means no more chars, plain
        // and simple
        if(remaining() == 0)
            return traits_type::eof();

        const char_type ch = *base_type::_str.clock(in_base_type::pos(), 1);
        base_type::_str.cunlock();
        return ch;
    }

    // NOTE: This leaves things unlocked, so only enable this for layer1-layer3 strings
    // this implicitly is the case as we do not implement 'data()' except for scenarios
    // where locking/unlocking is a noop (or otherwise inconsequential)
    char_type* gptr() { return base_type::_str.data() + in_base_type::pos(); }
};

}}}