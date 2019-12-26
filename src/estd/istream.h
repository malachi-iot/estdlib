#ifndef UTIL_EMBEDDED_ISTREAM_H
#define UTIL_EMBEDDED_ISTREAM_H

#include "streambuf.h"
#include "ios.h"
//#include "features.h"
#include "algorithm.h"
#include "traits/char_traits.h"

//#include <cassert>

#ifdef FEATURE_FRAB
#include <frab/systime.h>
#endif

// FIX: Re-include this if
//  a) we're sure Arduino and other off-the-beaten-std path has it and
//  b) we feel like fighting with the standard std namespace (which algorithm seems to auto include
//     i.e. it seems to specify 'using namespace std'
//#include <algorithm> // for min function

#define FEATURE_ESTD_IOS_GCOUNT

namespace estd {

/*
#ifdef min
#undef min
#endif

// lifted from http://www.cplusplus.com/reference/algorithm/min/
// FIX: put this into algorithm.h if we're gonna really roll with it
template <class T> const T& min (const T& a, const T& b)
{
    return !(b<a)?a:b;     // or: return !comp(b,a)?a:b; for version (2)
}*/

namespace internal {

//template<class TChar, class Traits = std::char_traits<TChar>>
template <class TStreambuf, class TBase = basic_ios<TStreambuf> >
class basic_istream : public
#ifdef FEATURE_IOS_STREAMBUF_FULL
        virtual
#endif
        TBase
{
    typedef TBase base_t;

public:
    typedef typename base_t::char_type char_type;
    typedef typename base_t::streambuf_type streambuf_type;
    typedef typename streambuf_type::off_type off_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::int_type int_type;

private:
    inline int_type standard_peek()
    {
#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif
        return this->good() ? this->rdbuf()->sgetc() : traits_type::eof();
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

#ifdef FEATURE_ESTD_IOS_GCOUNT
    streamsize _gcount = 0;

    typedef basic_istream<TStreambuf, TBase> __istream_type;

    // just a formality for now, to prep for if we ever want a real sentry
    struct sentry
    {
        typedef typename streambuf_type::traits_type traits_type;
        __istream_type& is;

        sentry(__istream_type& is, bool noskipws = false) : is(is) {}

        operator bool() const
        {
            return is.good();
        }
    };

public:
    streamsize gcount() const { return _gcount; }
#endif

public:
    int_type get()
    {
#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = 1;
#endif
        return this->rdbuf()->sbumpc();
    }

    // UNTESTED
    __istream_type& unget()
    {
#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif
        if(this->rdbuf()->sungetc() == traits_type::eof())
            this->setstate(ios_base::badbit);

        return *this;
    }

    // UNTESTED
    __istream_type& seekg(off_type off, ios_base::seekdir dir)
    {
        this->unsetstate(ios_base::failbit);
        this->rdbuf()->pubseekoff(off, dir, ios_base::in);
    }


    // nonblocking read
    // Only lightly tested
    streamsize readsome(char_type* s, streamsize count)
    {
        streambuf_type& rdbuf = *(this->rdbuf());
        // if count > number of available bytes
        // then read only available bytes
        // otherwise read all of count
        streamsize m = estd::min(count, rdbuf.in_avail());

#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = m;
#endif
        return rdbuf.sgetn(s, m);
    }

    __istream_type& read(char_type* s, streamsize n)
    {
        // TODO: optimization point.  We want to do something
        // so that we don't inline this (and other read/write operations like it)
        // all over the place
        if(this->rdbuf()->sgetn(s, n) != n)
            // TODO: Consider setting _gcount here to what *was* returned
            this->setstate(base_t::eofbit);

#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = n;
#endif

        return *this;
    }

    // TODO: optimize, ensure this isn't inlined
    __istream_type& getline(char_type* s, streamsize count, char_type delim = '\n')
    {
        streambuf_type* stream = this->rdbuf();

#ifdef FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif

        for(;;)
        {
            int_type c = stream->sbumpc();

            if(traits_type::eq(c, traits_type::eof()))
            {
                this->setstate(base_t::eofbit);
                break;
            }

            if(!count-- || traits_type::eq(c, delim))
            {
                this->setstate(base_t::failbit);
                break;
            }

            *s++ = c;
#ifdef FEATURE_ESTD_IOS_GCOUNT
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
#ifdef FEATURE_ESTD_IOS_SPEEKC
        // calling non-standard rdbuf()->speekc()
        return this->good() ? this->rdbuf()->speekc() : traits_type::eof();
#else
        if(this->rdbuf()->in_avail())
        {
            return standard_peek();
        }
        else
        {
#ifdef FEATURE_IOS_EXPERIMENTAL_TRAIT_NODATA
            return traits_type::nodata();
#else
#warning "eof used to indicate non-eof lack of data condition.   Not advised!"
            return traits_type::eof();
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
        while(count--)
        {
            int_type ch = get();

            if(ch == traits_type::eof())
            {
                this->setstate(ios_base::eofbit);
                return *this;
            }

#ifdef FEATURE_ESTD_IOS_GCOUNT
            _gcount++;
#endif
        }
        return *this;
    }

    // http://en.cppreference.com/w/cpp/io/basic_istream/ignore
    basic_istream& ignore(streamsize count, const int_type delim)
    {
        // "This [delimiter] test is disabled if delim is Traits::eof()"
        if(delim == traits_type::eof()) return ignore(count);

        while(count--)
        {
            int_type ch = get();

            if(ch == traits_type::eof())
            {
                this->setstate(ios_base::eofbit);
                break;
            }
            else if(ch == delim) break;

#ifdef FEATURE_ESTD_IOS_GCOUNT
            _gcount++;
#endif
        }
        return *this;
    }


    basic_istream& sync()
    {
        if(this->rdbuf()->pubsync() == -1)
            this->setstate(base_t::badbit);

        return *this;
    }

    basic_istream& operator>>(basic_istream& (*__pf)(basic_istream&))
    {
        return __pf(*this);
    }


#ifndef FEATURE_IOS_STREAMBUF_FULL
    //typedef typename base_t::stream_type stream_t;

    //basic_istream(stream_t& stream) : base_t(stream) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    template <class ... TArgs>
    basic_istream(TArgs&&...args) :
        base_t(std::forward<TArgs>(args)...) {}

    basic_istream(streambuf_type&& streambuf) :
        base_t(std::move(streambuf)) {}
#endif
    basic_istream(streambuf_type& streambuf) : base_t(streambuf) {}
#endif
};

}


#ifdef ESTD_POSIX
template<class TChar, class Traits = std::char_traits<TChar> >
using posix_istream = internal::basic_istream< posix_streambuf<TChar, Traits> >;

typedef posix_istream<char> istream;
#endif


// To change delimiters, we'll need to do something like this:
// http://stackoverflow.com/questions/7302996/changing-the-delimiter-for-cin-c
/*
inline basic_istream<char>& operator >>(basic_istream<char>& in, short& value)
{
    return *in;
}
*/

// NOTE: Due to C++ rules disallowing partial function specialization, we can't do
// a prototype here.  I would prefer specialization
// so that we can link in specialized implementations instead of forcing inline-includes
// but inline-includes are reasonable
// discussed here https://www.fluentcpp.com/2017/08/15/function-templates-partial-specialization-cpp/
//template <class TImpl, class TBase, class T>
//internal::basic_istream<TImpl, TBase>& operator >>(internal::basic_istream<TImpl, TBase>& in,
//                                                   T& value);


/**
 * Consume whitespace
 * @tparam TChar
 * @param __os
 * @return
 *
 * Compiles but not runtime tested
 */
template <class TStreambuf>
inline internal::basic_istream<TStreambuf>& ws(internal::basic_istream<TStreambuf>& __is)
{
    experimental::locale loc = __is.getloc();

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

// Experimental because:
// - naming I'm 90% on, not 100%
// - spans prefer to be uint8_t, streams prefer char
namespace experimental {
typedef estd::internal::streambuf<estd::internal::impl::in_span_streambuf<char> > ispanbuf;

typedef estd::internal::basic_istream<ispanbuf> ispanstream;
}


}
#endif //UTIL_EMBEDDED_ISTREAM_H
