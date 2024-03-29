#pragma once

#include "../streambuf.h"
#include "helpers.h"

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
    index_type _pos;

#ifdef __cpp_rvalue_references
    explicit pos_streambuf_base(index_type&& pos) : _pos(std::move(pos)) {}
#endif
    EXPLICIT pos_streambuf_base(const index_type& pos) : _pos(pos) {}

    inline const index_type& seekpos(const pos_type& p)
    {
        _pos = p;
        return _pos;
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
                _pos += off;
                return pos();

            default:
                return pos_type(off_type(-1));
        }
    }

public:
    // NOTE: Old comment here, just for reference as to why we have both index_type
    // and pos_type
    // This method in particular is sensitive to pos_type reference.  Stack usage goes
    // sky high if we return a copy

    ESTD_CPP_CONSTEXPR_RET const index_type& pos() const { return _pos; }
};

template <typename CharTraits, class Index = unsigned short>
struct in_pos_streambuf_base : pos_streambuf_base<CharTraits, Index>
{
    typedef pos_streambuf_base<CharTraits, Index> base_type;
    using typename base_type::traits_type;
    typedef typename base_type::index_type index_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;

#ifdef FEATURE_CPP_MOVESEMANTIC
    EXPLICIT in_pos_streambuf_base(index_type&& pos) : base_type(std::move(pos)) {}
#endif
    EXPLICIT in_pos_streambuf_base(const index_type& pos = 0) : base_type(pos) {}

protected:
    void gbump(int count) { this->_pos += count; }

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
    typedef pos_streambuf_base<CharTraits, Index> base_type;
    using typename base_type::traits_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;
    typedef typename base_type::index_type index_type;

#ifdef FEATURE_CPP_MOVESEMANTIC
    EXPLICIT out_pos_streambuf_base(index_type&& pos) : base_type(std::move(pos)) {}
#endif
    EXPLICIT out_pos_streambuf_base(const index_type& pos = 0) : base_type(pos) {}

protected:
    void pbump(int count) { this->_pos += count; }

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
