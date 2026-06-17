#pragma once

#include "../streambuf.h"
#include "pos.h"

#include "../../macro/push.h"

namespace estd { namespace internal { namespace impl {

// just the fundamental pieces, overflow/sync device handling will have to
// be implemented in a derived class
template <class CharTraits, size_t Extent = detail::dynamic_extent::value,
    class Base = estd::experimental::instance_provider<
        estd::span<typename CharTraits::char_type, Extent> > >
struct out_span_streambuf :
    out_pos_streambuf_base<CharTraits,
        typename Base::value_type::size_type >,
    Base
{
    using base_type = Base;
    using span_type = typename base_type::value_type;
    using size_type = typename span_type::size_type;
    using base_out_type = out_pos_streambuf_base<CharTraits, size_type>;
    using typename base_out_type::traits_type;
    using char_type = typename traits_type::char_type;

    typedef typename base_out_type::off_type off_type;
    typedef typename base_out_type::pos_type pos_type;
    typedef typename traits_type::int_type int_type;

    ESTD_CPP_CONSTEXPR(14) span_type& out() { return base_type::value(); }
    constexpr const span_type& out() const { return base_type::value(); }

    // NOTE: Would use Extent here but that breaks it for scenarios
    // where Extent == -1
    template <std::size_t N>
    explicit out_span_streambuf(char_type (&array)[N]) :
        base_type(array)
    {

    }

    template <class ...Args>
    constexpr explicit out_span_streambuf(Args&&...args) :
        base_type(std::forward<Args>(args)...)
    {

    }

    ESTD_CPP_CONSTEXPR(14) char_type* pbase() const
    {
        return const_cast<char_type*>(out().data());
    }

    ESTD_CPP_CONSTEXPR(14) char_type* pptr() const { return pbase() + base_out_type::pos(); }
    ESTD_CPP_CONSTEXPR(14) char_type* epptr() const { return pbase() + out().size_bytes(); }

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

        // DEBT: Do bounds check if flagged by strictness feature flag

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

    // DEBT: Put in a seekpos pseudo-override to do bounds check if feature flag indicates to do so

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
template <class CharTraits,
        size_t Extent = detail::dynamic_extent::value,
        class Base = estd::experimental::instance_provider<estd::span<typename CharTraits::char_type, Extent> > >
struct in_span_streambuf :
        in_pos_streambuf_base<CharTraits>,

        streambuf_gptr_tag,

        Base
{
    using base_type = Base;
    typedef in_pos_streambuf_base<CharTraits> base_pos_type;

    using typename base_pos_type::traits_type;
    using base_pos_type::pos;

    using char_type = typename traits_type::char_type;
    typedef typename base_type::value_type span_type;
    typedef typename span_type::size_type size_type;
    typedef typename traits_type::int_type int_type;
    typedef typename base_pos_type::pos_type pos_type;
    typedef typename remove_const<char_type>::type nonconst_char_type;

#if FEATURE_ESTD_STREAMBUF_POLICY
    struct policy : base_type::policy
    {
        using rfc = internal::rfc::rfc2119;

        struct use : base_type::policy::use
        {
            static constexpr rfc gptr = rfc::should;
            static constexpr rfc seekoff = rfc::may;
            static constexpr rfc seekpos = rfc::may;
        };
    };
#endif

protected:
    constexpr const span_type& in() const { return base_type::value(); }

public:
    template <class ...Args>
    constexpr explicit in_span_streambuf(Args&&...args)
        : base_type(std::forward<Args>(args)...)
    {

    }

    constexpr char_type* eback() const { return in().data(); }
    constexpr char_type* gptr() const { return eback() + pos(); }
    constexpr char_type* egptr() const { return eback() + in().size(); }

protected:
    constexpr streamsize xin_avail() const
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

    constexpr const char_type& xsgetc() const { return *gptr(); }
};

// EXPERIMENTAL
// Works-ish, but no use case identified
#if __cpp_deduction_guides && UNUSED
template <class CharTraits, size_t Extent>
in_span_streambuf(estd::span<typename CharTraits::char_type, Extent>) ->
    in_span_streambuf<CharTraits, Extent>;
#endif



}}}

#include "../../macro/pop.h"
