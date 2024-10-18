#pragma once

#include "../streambuf.h"
#include "pos.h"

#include "../../macro/push.h"

namespace estd { namespace internal { namespace impl {

// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
// TODO: Do char_traits the same between this and in_span_streambuf
template <class T, estd::size_t Extent = detail::dynamic_extent::value,
        class Base = estd::experimental::instance_provider<estd::span<T, Extent> > >
struct out_span_streambuf :
        out_pos_streambuf_base<char_traits<T>,
            typename Base::value_type::size_type >,
        Base
{
    typedef Base base_type;
    typedef T char_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef out_pos_streambuf_base<char_traits<T>, size_type> base_out_type;
    using typename base_out_type::traits_type;
    typedef typename base_out_type::off_type off_type;
    typedef typename base_out_type::pos_type pos_type;
    typedef typename traits_type::int_type int_type;

    span_type& out() { return base_type::value(); }
    const span_type& out() const { return base_type::value(); }

    /*
    out_span_streambuf(T* buf, size_type size) :
            base_type(span_type(buf, size))
    {

    } */

    // NOTE: Would use Extent here but that breaks it for scenarios
    // where Extent == -1
    template <std::size_t N>
    out_span_streambuf(char_type (&array)[N]) :
            base_type(array)
    {

    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    out_span_streambuf(span_type&& move_from) :
        base_type(std::move(move_from))
    {

    }
#endif

    out_span_streambuf(const span_type& copy_from) :
            base_type(copy_from)
    {

    }


    out_span_streambuf(char_type* data, const pos_type& count) :
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

    int_type sputc(char_type ch)
    {
        const pos_type& pos = base_out_type::pos();
        if(pos >= out().size_bytes())
            return base_out_type::overflow();

        *(pptr()) = ch;
        this->pbump(1);
        return traits_type::to_int_type(ch);
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
                base_out_type::seekpos(off);
                break;

            case ios_base::end:
                // UNTESTED
                base_out_type::seekpos(out().size_bytes() + off);
                break;
        }

        return base_out_type::pos();
    }

public:
    void setbuf_experimental(char_type* s, streamsize n)
    {
        base_out_type::seekpos(0);
        base_type::value(span<char_type>(s, n));
    }
};


// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
// DEBT: Refactor this to take CharTraits directly
template <class TChar,
        class TCharTraits =  estd::char_traits<TChar>,
        std::size_t Extent = detail::dynamic_extent::value,
        class TBase = estd::experimental::instance_provider<estd::span<TChar, Extent> > >
struct in_span_streambuf :
        in_pos_streambuf_base<TCharTraits>,

        streambuf_gptr_tag,

        TBase
{
    typedef in_pos_streambuf_base<TCharTraits> base_pos_type;
    typedef TBase base_type;

    using typename base_pos_type::traits_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_pos_type::pos_type pos_type;
    typedef TChar char_type;
    typedef typename remove_const<char_type>::type nonconst_char_type;

protected:
    const span_type& in() const { return base_type::value(); }

    ESTD_CPP_CONSTEXPR_RET const pos_type& pos() const
    {
        return base_pos_type::pos();
    }

public:
    in_span_streambuf(const estd::span<TChar, Extent>& copy_from)
        : base_type(copy_from)
    {

    }

    ESTD_CPP_CONSTEXPR_RET char_type* eback() const { return in().data(); }
    ESTD_CPP_CONSTEXPR_RET char_type* gptr() const { return eback() + pos(); }
    ESTD_CPP_CONSTEXPR_RET char_type* egptr() const { return eback() + in().size(); }

protected:
    ESTD_CPP_CONSTEXPR_RET streamsize xin_avail() const
    {
        return in().size() - pos();
    }

    streamsize showmanyc() const { return base_pos_type::showmanyc(xin_avail()); }

    streamsize xsgetn(nonconst_char_type* s, streamsize count)
    {
        // NOTE: No uflow/eof handling since a span unlike a netbuf is just one buffer and that's it
        const streamsize c = estd::min(count, xin_avail());
        estd::copy_n(gptr(), c, s);
        this->gbump(c);
        return c;
    }

    ESTD_CPP_CONSTEXPR_RET const char_type& xsgetc() const { return *gptr(); }
};


}}}

#include "../../macro/pop.h"