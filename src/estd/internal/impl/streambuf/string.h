#pragma once

#include "../streambuf.h"

// Since strings always have an "out" component (track how many characters are written to
// it) we don't have a distinct "in" string buf.
namespace estd { namespace internal { namespace impl {

template <class TString>
struct stringbuf_base : streambuf_base<typename TString::traits_type>
{
    stringbuf_base() {}
};

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

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    out_stringbuf(TArgs&&...args) :
        _str(std::forward<TArgs>(args)...)
    {}
#else
    out_stringbuf() {}

    template <class TParam1>
    out_stringbuf(TParam1& p) : _str(p) {}
#endif

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
struct basic_stringbuf :
        out_stringbuf<TString>,
        in_pos_streambuf_base<typename TString::traits_type>,

        streambuf_sungetc_tag
{
    typedef out_stringbuf<TString> base_type;
    typedef typename base_type::traits_type traits_type;
    typedef in_pos_streambuf_base<traits_type> in_base_type;
    typedef typename base_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_type::string_type string_type;
    typedef typename string_type::size_type size_type;
    typedef typename base_type::off_type off_type;
    typedef typename base_type::pos_type pos_type;

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_stringbuf(TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...)
    {}
#else
    basic_stringbuf() {}

    template <class TParam1>
    basic_stringbuf(TParam1& p) :
            base_type(p)
    {}
#endif

    // See in_base_type::pos() for why we use ref here
    const typename in_base_type::pos_type& pos() const { return in_base_type::pos(); }

    streamsize xsgetn(char_type* s, streamsize count)
    {
        size_type count_copied = base_type::str().copy(s, count, pos());

        in_base_type::gbump(count_copied);

        return count_copied;
    }

    size_type xin_avail() const
    {
        return this->_str.length() - pos();
    }

    streamsize showmanyc() const { return in_base_type::showmanyc(xin_avail()); }


    char_type xsgetc() const
    {
        // DEBT: May be better off using standard string indexer here.  Its fancy iterator probably
        // will optimize out
        char_type ch = *base_type::_str.clock(pos(), 1);
        base_type::_str.cunlock();
        return ch;
    }

    // NOTE: This leaves things unlocked, so only enable this for layer1-layer3 strings
    // this implicitly is the case as we do not implement 'data()' except for scenarios
    // where locking/unlocking is a noop (or otherwise inconsequential)
    //char_type* gptr() { return base_type::_str.data() + in_base_type::pos(); }


    // UNTESTED
    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which)
    {
        switch(dir)
        {
            case ios_base::beg:
                if(which & ios_base::in)
                    return in_base_type::seekpos(off);
                break;

            case ios_base::cur:
                if(which & ios_base::in)
                    in_base_type::gbump(off);
                break;

            case ios_base::end:
                if(which & ios_base::in)
                    return in_base_type::seekpos(this->_str.length() + off);
                break;
        }

        return pos_type(off_type(-1));
    }

    int_type sungetc()
    {
        if(pos() == 0)
            return this->pbackfail();
        else
        {
            in_base_type::gbump(-1);
            return traits_type::to_int_type(xsgetc());
        }
    }
};

}}}