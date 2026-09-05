#pragma once

#include "base.h"
#include "pos.h"

// Since strings always have an "out" component (track how many characters are written to
// it) we don't have a distinct "in" string buf.
namespace estd { namespace internal { namespace impl {

template <ESTD_CPP_CONCEPT(concepts::v1::String) String>
struct stringbuf_base : streambuf_base<typename remove_reference_t<String>::traits_type>
{
    ESTD_CPP_DEFAULT_CTOR(stringbuf_base)

    using string_type = remove_reference_t<String>;
};

// 17JUN26 - No seek support at this time
template <class String>
struct out_stringbuf : stringbuf_base<String>
{
    using base_type = stringbuf_base<String>;
    using typename base_type::string_type;

    typedef typename string_type::value_type char_type;
    // NOTE: Beware, this is acting a little differently than regular char_traits
    //typedef typename string_type::traits_type traits_type;
    using typename base_type::traits_type;
    typedef typename traits_type::off_type off_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::int_type int_type;

    String str_;

    ESTD_CPP_FORWARDING_CTOR_MEMBER(out_stringbuf, str_)

#if FEATURE_ESTD_STREAMBUF_POLICY
    struct policy : base_type::policy
    {
        using rfc = internal::rfc::rfc2119;

        struct use
        {
            static constexpr rfc gptr = rfc::must_not;
            static constexpr rfc pptr = rfc::should_not;
            static constexpr rfc seekoff = rfc::should_not;
            static constexpr rfc seekpos = rfc::must_not;
        };
    };
#endif

    ESTD_CPP_CONSTEXPR(17) streamsize xsputn(const char_type* s, streamsize count)
    {
        int remaining_free = str_.max_size() - str_.size();
        if(count > remaining_free)  count = remaining_free;
        // DEBT: normal strings throw an exception if we exceed internal
        // buffer size.  'append' just asserts, thus the extra check above.
        // ideally we'd have an augmented append to also choose an estd::expected result
        str_.append(s, count);
        return count;
    }

    ESTD_CPP_CONSTEXPR(17) int_type sputc(char_type ch)
    {
        str_ += ch;
        return traits_type::to_int_type(ch);
    }

    char_type* pbase() { return str_.data(); }
    char_type* pptr() { return str_.data() + str_.size(); }
    char_type* egptr() { return str_.data() + str_.max_size(); }

    // deviates from spec in that this is NOT a copy, but rather a direct reference
    // to the tracked string.  Take care
    constexpr const string_type& str() const { return str_; }

    void resize(unsigned sz) { str_.resize(sz); }

    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which)
    {
        // Doesn't really support seeking, but can fake it to tell you strlen at least
        if(off == 0 && ios_base::end && which == ios_base::out)  return str_.size();

        return pos_type(off_type(-1));
    }

    constexpr typename string_type::view_type view() const
    {
        return str_;
    }

    // non-standard, won't work with all varieties of estd::string
    void clear()
    {
        str_.clear();
    }
};


// Implicitly basic_istringbuf, but does some out basic_ostringbuf things too
template <class String>
struct basic_stringbuf :
        out_stringbuf<String>,
        in_pos_streambuf_base<typename remove_reference_t<String>::traits_type>,

        streambuf_sungetc_tag
{
    typedef out_stringbuf<String> base_type;
    using char_traits = typename remove_reference_t<String>::traits_type;
    using in_base_type = in_pos_streambuf_base<char_traits>;
    using typename in_base_type::index_type;
    using typename base_type::char_type;
    using nonconst_pointer = estd::remove_const_t<char_type>*;

    typedef typename base_type::traits_type traits_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_type::string_type string_type;
    typedef typename string_type::size_type size_type;
    typedef typename base_type::off_type off_type;
    typedef typename base_type::pos_type pos_type;

    using base_type::str_;
    using in_base_type::pos;
    using typename base_type::policy;

    ESTD_CPP_FORWARDING_CTOR(basic_stringbuf)

    streamsize xsgetn(nonconst_pointer s, streamsize count)
    {
        size_type count_copied = str_.copy(s, count, pos());

        in_base_type::gbump(count_copied);

        return count_copied;
    }

    constexpr size_type xin_avail() const
    {
        return str_.length() - pos();
    }

    ESTD_CPP_CONSTEXPR(14) index_type seekpos(const pos_type& p,
        ios_base::openmode which)
    {
        // DEBT: Only 'in' seeking supported
        if(which & ios_base::out)    return -1;

        return in_base_type::seekpos(p);
    }

    streamsize showmanyc() const { return in_base_type::showmanyc(xin_avail()); }


    char_type xsgetc() const
    {
        // DEBT: May be better off using standard string indexer here.  Its fancy iterator probably
        // will optimize out
        char_type ch = *str_.clock(pos(), 1);
        str_.cunlock();
        return ch;
    }

    ESTD_CPP_CONSTEXPR(14) char_type* eback()
    {
        return str_.data();
    }

    ESTD_CPP_CONSTEXPR(14) char_type* gptr()
    {
        return str_.data() + pos();
    }

    // DEBT: We still need an egptr(), just unclear whether that ought to be
    // where null termination lives or max string


    pos_type seekoff(off_type off, ios_base::seekdir dir, ios_base::openmode which) // NOLINT
    {
        // DEBT: Only 'in' seeking supported
        if(which & ios_base::out)    return -1;

        return in_base_type::seekoff(off, dir);
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
