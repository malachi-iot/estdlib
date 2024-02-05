#pragma once

#include "base.h"
#include "../../fwd/string.h"

// Ironic, we have to put the true-blue pseudo-double-buffer that streambuf represents into a distinct
// 'buffer' area

namespace estd { namespace internal { namespace impl {

#if __cplusplus >= 201103L
// This class has nothing specific inhibiting c++03 compat, I am just saving time.
// Hard wired to layer1 string. a span version of this would be nice too
// DEBT: It's likely we can combine this with existing span buf and stringbuf to have a noop-Streambuf
// in them
template <class Streambuf, unsigned len = 64>
class out_buffered_stringbuf : public streambuf_base<typename Streambuf::traits_type>
{
    using base_type = streambuf_base<typename Streambuf::traits_type>;

    // TODO: Make this an is-a not a has-a so we can do both istreambuf and ostreambuf
    Streambuf streambuf_;

    static constexpr int len_ = len;

    // TODO: Make this a string type we pass in somehow
    estd::layer1::string<len_, false> buf_;

    //constexpr
    unsigned remaining() const
    {
        unsigned m = buf_.max_size();
        unsigned s = buf_.size();
        //return buf_.max_size() - buf_.size();
        return m - s;
    }

    // DEBT: Finish up string/allocated_array underlying full implementation
    constexpr bool full() const
    {
        return remaining() == 0;
    }

    int ll_sync()
    {
        streambuf_.xsputn(buf_.data(), buf_.size());
        buf_.clear();
        return 0;
    }

public:
    using typename base_type::int_type;
    using typename base_type::char_type;
    using typename base_type::traits_type;

    Streambuf& rdbuf() { return streambuf_; }
    const Streambuf& rdbuf() const { return streambuf_; }

protected:
    int sync()
    {
        if(buf_.empty()) return 0;

        return ll_sync();
    }

    int_type overflow(int_type ch)
    {
        if(full())
        {
            if(ll_sync() != 0)  return traits_type::eof();
        }

        if(traits_type::not_eof(ch))
        {
            buf_ += traits_type::to_char_type(ch);
        }

        // "Returns unspecified value not equal to Traits::eof() on success"
        // DEBT: In some universe, '*' might be EOF, so watch out here
        return '*';
    }

    estd::streamsize xsputn(const char_type* s, estd::streamsize count)
    {
        const estd::streamsize maximum = remaining();

        buf_.append(s, count > maximum ? maximum : count);

        // wrapper sputn handles this via 'overflow' - a little clumsy though
        //if(full())
        //    ll_sync();

        return count;
    }

public:
    ESTD_CPP_FORWARDING_CTOR_MEMBER(out_buffered_stringbuf, streambuf_)

    char_type* pbase() const { return buf_.data(); }
    char_type* pptr() const { return pbase() + buf_.size(); }
    char_type* epptr() const { return pbase() + buf_.max_size(); }

    int_type sputc(char_type ch)
    {
        return overflow(ch);
    }
};



#endif

}}}