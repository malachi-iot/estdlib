#pragma once

#include "../streambuf.h"
#include "helpers.h"

namespace estd { namespace internal { namespace impl {

template <typename TCharTraits>
struct pos_streambuf_base : streambuf_base<TCharTraits>
{
    typedef TCharTraits traits_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

    pos_type _pos;

    pos_streambuf_base(pos_type pos) : _pos(pos) {}

    pos_type pos() const { return _pos; }

    // FIX: Need this because ESTD_FN_HAS_METHOD falls on its face for detecting overloaded methods
    pos_type get_pos() const { return pos(); }

protected:
    // DEBT: This is obsolete, seekpos is the standard way
    void pos(pos_type p) { _pos = p; }

    inline pos_type seekpos(pos_type p)
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
};

template <typename TCharTraits>
struct in_pos_streambuf_base : pos_streambuf_base<TCharTraits>
{
    typedef TCharTraits traits_type;
    typedef pos_streambuf_base<traits_type> base_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

    in_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void gbump(int count) { this->_pos += count; }

    /* Almost works, not quite ready
    inline pos_type seekoff(off_type off, ios_base::seekdir way,
                            ios_base::openmode which = ios_base::in | ios_base::out)
    {
        if(which != ios_base::in) return pos_type(off_type(-1));

        return base_type::seekoff(off, way);
    }
     */
};


template <typename TCharTraits>
struct out_pos_streambuf_base : pos_streambuf_base<TCharTraits>
{
    typedef pos_streambuf_base<TCharTraits> base_type;
    typedef TCharTraits traits_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

    out_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void pbump(int count) { this->_pos += count; }
};

}}}