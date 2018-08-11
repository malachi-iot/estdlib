#ifndef UTIL_EMBEDDED_ISTREAM_H
#define UTIL_EMBEDDED_ISTREAM_H

#include "streambuf.h"
#include "ios.h"
#include "features.h"

//#include <cassert>

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif

// FIX: Re-include this if
//  a) we're sure Arduino and other off-the-beaten-std path has it and
//  b) we feel like fighting with the standard std namespace (which algorithm seems to auto include
//     i.e. it seems to specify 'using namespace std'
//#include <algorithm> // for min function

#define FEATURE_IOS_GCOUNT

namespace FactUtilEmbedded { namespace std {

#ifdef min
#undef min
#endif

// lifted from http://www.cplusplus.com/reference/algorithm/min/
// FIX: put this into algorithm.h if we're gonna really roll with it
template <class T> const T& min (const T& a, const T& b)
{
    return !(b<a)?a:b;     // or: return !comp(b,a)?a:b; for version (2)
}

template<class TChar, class Traits = char_traits<TChar>>
class basic_istream :
#ifdef FEATURE_IOS_STREAMBUF_FULL
        virtual
#endif
        public basic_ios<TChar, Traits>
{
    typedef basic_ios<TChar> base_t;
    typedef TChar char_type;
    typedef typename base_t::basic_streambuf_t basic_streambuf_t;
    typedef typename Traits::int_type int_type;

    inline int_type standard_peek()
    {
        return this->good() ? this->rdbuf()->sgetc() : Traits::eof();
    }

    /**
     *
     * @return true if data available, false if timeout occured
     */
    bool block_experimental()
    {
        // TODO: add timeout logic here
        while(!this->rdbuf()->in_avail());

        return true;
    }

#if defined(FEATURE_FRAB)
    /**
     *
     * @return true if character available, false if timeout and no character available
     */
    bool block_timeout(uint16_t timeout_ms)
    {
        uint32_t timeout_absolute =  framework_abstraction::millis() + timeout_ms;
        while(framework_abstraction::millis() < timeout_absolute)
        {
            if(this->rdbuf()->in_avail()) return true;
        }

        return false;
    }
#endif

#ifdef FEATURE_IOS_GCOUNT
    streamsize _gcount = 0;

public:
    streamsize gcount() const { return _gcount; }
#endif

public:
    typedef basic_istream<TChar, Traits> __istream_type;

    int get()
    {
        return this->rdbuf()->sbumpc();
    }


    // nonblocking read
    // UNTESTED
    streamsize readsome(TChar* s, streamsize count)
    {
        auto rdbuf = *(this->rdbuf());
        // if count > number of available bytes
        // then read only available bytes
        // otherwise read all of count
        streamsize m = min(count, rdbuf.in_avail());

        return rdbuf.sgetn(s, m);
    }

    __istream_type& read(TChar* s, streamsize n)
    {
        // TODO: optimization point.  We want to do something
        // so that we don't inline this (and other read/write operations like it)
        // all over the place
        if(this->rdbuf()->sgetn(s, n) != n)
            this->setstate(base_t::eofbit);

        return *this;
    }

    // TODO: optimize, ensure this isn't inlined
    __istream_type& getline(char_type* s, streamsize count, char_type delim = '\n')
    {
        basic_streambuf_t* stream = this->rdbuf();

#ifdef FEATURE_IOS_GCOUNT
        _gcount = 0;
#endif

        for(;;)
        {
            int_type c = stream->sbumpc();

            if(Traits::eq(c, Traits::eof()))
            {
                this->setstate(base_t::eofbit);
                break;
            }

            if(!count-- || Traits::eq(c, delim))
            {
                this->setstate(base_t::failbit);
                break;
            }

            *s++ = c;
#ifdef FEATURE_IOS_GCOUNT
            _gcount++;
#endif
        }

        *s = 0;

        return *this;
    }

    /**
     * The proper behavior (imo) of 'peek' is to permit blocking, so this non-standard
     * method *always* is nonblocking version of peek.
     * @return
     */
#ifdef FEATURE_IOS_EXPERIMENTAL_GETSOME
    int_type getsome()
    {
#ifdef FEATURE_IOS_SPEEKC
        // TODO: change this to call non-standard this->rdbuf()->speekc();
        return this->good() ? this->rdbuf()->speekc() : Traits::eof();
#else
        if(this->rdbuf()->in_avail())
        {
            return standard_peek();
        }
        else
        {
#ifdef FEATURE_IOS_EXPERIMENTAL_TRAIT_NODATA
            return Traits::nodata();
#else
#warning "eof used to indicate non-eof lack of data condition.   Not advised!"
            return Traits::eof();
#endif
        }
#endif
    }
#endif

#if defined(FEATURE_FRAB) && defined(FEATURE_IOS_TIMEOUT)
    int_type peek(uint16_t timeout)
    {
        // block based on a hardware timer
        if(block_timeout(timeout))
        {
            // character available
            return standard_peek();
        }
        else
        {
            return Traits::nodata();
        }
    }
#endif

    int_type peek()
    {
        return standard_peek();
    }

    // delim test is disabled if delim is default value, which would be EOF
    basic_istream& ignore(streamsize count = 1)
    {
        while(count--) get();
        return *this;
    }

    // http://en.cppreference.com/w/cpp/io/basic_istream/ignore
    basic_istream& ignore(streamsize count, const int_type delim)
    {
        if(delim == Traits::eof()) return ignore(count);

        while(count--)
        {
            int_type ch = get();
            if(ch == delim) break;
        }
        return *this;
    }

    basic_istream& operator>>(basic_istream& (*__pf)(basic_istream&))
    {
        return __pf(*this);
    }


#ifndef FEATURE_IOS_STREAMBUF_FULL
    typedef typename base_t::stream_t stream_t;

    //basic_istream(stream_t& stream) : base_t(stream) {}

    template <class _TStream, class ... TArgs>
    basic_istream(_TStream& stream, TArgs...args) : base_t(stream, args...) {}
#endif
};

typedef basic_istream<char> istream;


// To change delimiters, we'll need to do something like this:
// http://stackoverflow.com/questions/7302996/changing-the-delimiter-for-cin-c
/*
inline basic_istream<char>& operator >>(basic_istream<char>& in, short& value)
{
    return *in;
}
*/

#ifndef FEATURE_IOSTREAM

/**
 * Consume whitespace
 * @tparam TChar
 * @param __os
 * @return
 *
 * Compiles but not runtime tested
 */
template <class TChar>
inline basic_istream<TChar>& ws(basic_istream<TChar>& __is)
{
    auto loc = __is.getloc();

    // isspace will automatically fall out if it's an EOF (or nodata)
    for(;;)
    {
        int ch = __is.peek();
        if(isspace((char)ch, loc))
        {
            __is.ignore();
        }
        else
            return __is;
    }
}
#endif

} }
#endif //UTIL_EMBEDDED_ISTREAM_H