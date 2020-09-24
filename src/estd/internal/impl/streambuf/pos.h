#pragma once

#include "../streambuf.h"

namespace estd { namespace internal { namespace impl {

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
struct in_pos_streambuf_base :
        pos_streambuf_base<typename TCharTraits::pos_type>,
        streambuf_base<TCharTraits>
{
    typedef pos_streambuf_base<typename TCharTraits::pos_type> base_type;
    typedef typename base_type::pos_type pos_type;

    in_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void gbump(int count) { this->_pos += count; }
};


template <typename TCharTraits>
struct out_pos_streambuf_base :
        pos_streambuf_base<typename TCharTraits::pos_type>,
        streambuf_base<TCharTraits>
{
    typedef pos_streambuf_base<typename TCharTraits::pos_type> base_type;
    typedef typename base_type::pos_type pos_type;

    out_pos_streambuf_base(pos_type pos = 0) : base_type(pos) {}

protected:
    void pbump(int count) { this->_pos += count; }
};

}}}