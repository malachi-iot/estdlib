#pragma once

#include "base.h"

// Since strings always have an "out" component (track how many characters are written to
// it) we don't have a distinct "in" string buf.
namespace estd { namespace internal { namespace impl {

template <ESTD_CPP_CONCEPT(concepts::v1::String) String>
struct stringbuf_base : streambuf_base<typename remove_reference<String>::type::traits_type>
{
    ESTD_CPP_DEFAULT_CTOR(stringbuf_base)

    typedef typename remove_reference<String>::type string_type;
};

template <class String>
struct out_stringbuf : stringbuf_base<String>
{
    using base_type = stringbuf_base<String>;
    using typename base_type::string_type;

    typedef typename string_type::value_type char_type;
    // NOTE: Beware, this is acting a little differently than regular char_traits
    typedef typename string_type::traits_type traits_type;
    typedef typename traits_type::off_type off_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::int_type int_type;

    String str_;

    ESTD_CPP_FORWARDING_CTOR_MEMBER(out_stringbuf, str_)

    streamsize xsputn(const char_type* s, streamsize count)
    {
        // FIX: normal strings throw an exception if we exceed internal
        // buffer size, but here we should instead have an optional error
        // facility
        str_.append(s, count);
        return count;
    }

    int_type sputc(char_type ch)
    {
        str_ += ch;
        return traits_type::to_int_type(ch);
    }

    // deviates from spec in that this is NOT a copy, but rather a direct reference
    // to the tracked string.  Take care
    ESTD_CPP_CONSTEXPR_RET const string_type& str() const { return str_; }

    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which)
    {
        // TODO: check to make sure which contains ios_base::out
        return str_.size();
    }

    ESTD_CPP_CONSTEXPR_RET typename string_type::view_type view() const
    {
        return str_;
    }

    // non-standard, won't work with all varieties of estd::string
    void clear()
    {
        str_.clear();
    }
};


template <class String>
struct basic_stringbuf :
        out_stringbuf<String>,
        in_pos_streambuf_base<typename remove_reference_t<String>::traits_type>,

        streambuf_sungetc_tag
{
    typedef out_stringbuf<String> base_type;
    typedef typename base_type::traits_type traits_type;
    typedef in_pos_streambuf_base<traits_type> in_base_type;
    typedef typename base_type::char_type char_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_type::string_type string_type;
    typedef typename string_type::size_type size_type;
    typedef typename base_type::off_type off_type;
    typedef typename base_type::pos_type pos_type;

    ESTD_CPP_FORWARDING_CTOR(basic_stringbuf)

    // See in_base_type::pos() for why we use ref here
    const typename in_base_type::index_type& pos() const { return in_base_type::pos(); }

    streamsize xsgetn(char_type* s, streamsize count)
    {
        size_type count_copied = base_type::str().copy(s, count, pos());

        in_base_type::gbump(count_copied);

        return count_copied;
    }

    size_type xin_avail() const
    {
        return this->str_.length() - pos();
    }

    streamsize showmanyc() const { return in_base_type::showmanyc(xin_avail()); }


    char_type xsgetc() const
    {
        // DEBT: May be better off using standard string indexer here.  Its fancy iterator probably
        // will optimize out
        char_type ch = *base_type::str_.clock(pos(), 1);
        base_type::str_.cunlock();
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
                    return in_base_type::seekpos(this->str().length() + off);
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