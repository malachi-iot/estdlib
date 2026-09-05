#pragma once

#include "base.h"

namespace estd { namespace internal { namespace impl {

// NOTE: In esp-idf, pos_type is big and doesn't seem to play well
// with RVO - so returning/passing a "const pos_type&" rather than "pos_type"
// makes a difference
template <typename CharTraits, class Index = unsigned short>
struct pos_streambuf_base : streambuf_base<CharTraits>
{
    using base_type = streambuf_base<CharTraits>;
    using typename base_type::traits_type;
    typedef typename traits_type::int_type int_type;
    // FIX: Our pos_type here should instead by an unsigned
    // that likely a derived class specifies the bitness of
    //typedef typename traits_type::pos_type pos_type;
    typedef Index pos_type;
    // Not calling pos_type because that carries a C++ meaning of fpos, statefulness
    // and signed integer - none of which we want for our positioner
    typedef Index index_type;
    typedef typename traits_type::off_type off_type;

protected:
    index_type pos_;

    constexpr explicit pos_streambuf_base(index_type&& pos) : pos_(std::move(pos)) {}
    constexpr explicit pos_streambuf_base(const index_type& pos) : pos_(pos) {}

    ESTD_CPP_CONSTEXPR(14) const index_type& seekpos(const pos_type& p,
        ios_base::openmode = {})
    {
        return pos_ = p;
    }

    // DEBT: This is an underlying call, notice the lack of inspection of 'openmode'.  This in
    // it of itself is just fine.  However, further derived classes are responsible for
    // creating the dual-mode version.  We like this single-mode version since in embedded
    // scenarios it's nice to have a dedicated in/out positional streambuf
    inline pos_type seekoff(off_type off, ios_base::seekdir way,
        ios_base::openmode = ios_base::in | ios_base::out)
    {
        switch(way)
        {
            case ios_base::beg:
                return seekpos(off);

            case ios_base::cur:
                return pos_ += off;

            default:
                return pos_type(off_type(-1));
        }
    }

public:
    // NOTE: Old comment here, just for reference as to why we have both index_type
    // and pos_type
    // This method in particular is sensitive to pos_type reference.  Stack usage goes
    // sky high if we return a copy

    constexpr const index_type& pos() const { return pos_; }
};

template <typename CharTraits, class Index = unsigned short>
struct in_pos_streambuf_base : pos_streambuf_base<CharTraits, Index>
{
    typedef pos_streambuf_base<CharTraits, Index> base_type;
    using typename base_type::traits_type;
    typedef typename base_type::index_type index_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;

    constexpr explicit in_pos_streambuf_base(index_type&& pos) : base_type(std::move(pos)) {}
    constexpr explicit in_pos_streambuf_base(const index_type& pos = 0) : base_type(pos) {}

protected:
    ESTD_CPP_CONSTEXPR(14) void gbump(int count) { this->pos_ += count; }

    inline pos_type seekoff(off_type off, ios_base::seekdir way,
                            ios_base::openmode which = ios_base::in | ios_base::out)
    {
        // NOTE: We are permissive here to maintain compatibility with the spirit of
        // std library.  Otherwise, we would do which == ios_base::in
        if(which & ios_base::in) return base_type::seekoff(off, way);

        return pos_type(off_type(-1));
    }
};


template <typename CharTraits, class Index = unsigned short>
struct out_pos_streambuf_base : pos_streambuf_base<CharTraits, Index>
{
    using base_type = pos_streambuf_base<CharTraits, Index>;
    using typename base_type::traits_type;
    using typename base_type::pos_type;
    using typename base_type::off_type;
    using typename base_type::index_type;

    constexpr explicit out_pos_streambuf_base(index_type&& pos) : base_type(std::move(pos)) {}
    constexpr explicit out_pos_streambuf_base(const index_type& pos = 0) : base_type(pos) {}

protected:
    ESTD_CPP_CONSTEXPR(14) void pbump(int count) { this->pos_ += count; }

    inline pos_type seekoff(off_type off, ios_base::seekdir way,
                            ios_base::openmode which = ios_base::in | ios_base::out)
    {
        // NOTE: We are permissive here to maintain compatibility with the spirit of
        // std library.  Otherwise, we would do which == ios_base::in
        if(which & ios_base::out) return base_type::seekoff(off, way);

        return pos_type(off_type(-1));
    }
};

}}}
