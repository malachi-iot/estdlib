#pragma once

#include "ios_policy.h"
#include "ios.h"
#include "feature/ios.h"

#include "macro/push.h"


namespace estd { namespace detail {

template <ESTD_CPP_CONCEPT(concepts::v1::InStreambuf) Streambuf, class Base>
class basic_istream : public
#ifdef FEATURE_IOS_STREAMBUF_FULL
                      virtual
#endif
                      Base
{
    typedef Base base_type;

public:
    typedef typename base_type::char_type char_type;
    typedef typename base_type::streambuf_type streambuf_type;
    typedef typename streambuf_type::off_type off_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::int_type int_type;

    typedef typename base_type::locale_type locale_type;
    typedef typename base_type::blocking_type blocking_type;

private:
    inline int_type standard_peek()
    {
#if FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif
        return this->good() ?
               // Alas, standard C++ has set the precedent where peek sometimes blocks.
               // We can flip that behavior on and off at the istream level.
               blocking_type::sgetc(*this, this->rdbuf()) :
               traits_type::eof();
    }

    /**
     *
     * @return true if data available, false if timeout occured
     */
    bool block_experimental()
    {
        // TODO: add timeout logic here
        while (!this->rdbuf()->in_avail());

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

    typedef basic_istream<Streambuf, Base> __istream_type;

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

#if FEATURE_ESTD_IOS_GCOUNT
    streamsize _gcount;
    void gcount(streamsize value) { _gcount = value; }
public:
    streamsize gcount() const { return _gcount; }
#else
    void gcount(streamsize value) {}
#endif

public:
    int_type get()
    {
#if FEATURE_ESTD_IOS_GCOUNT
        ++_gcount;
#endif
        return this->rdbuf()->sbumpc();
    }

    // UNTESTED
    __istream_type& unget()
    {
#if FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif
        if (this->rdbuf()->sungetc() == traits_type::eof())
            this->setstate(ios_base::badbit);

        return *this;
    }

    __istream_type& seekg(off_type off, ios_base::seekdir dir)
    {
        this->unsetstate(ios_base::failbit);
        this->rdbuf()->pubseekoff(off, dir, ios_base::in);
        return *this;
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

#if FEATURE_ESTD_IOS_GCOUNT
        _gcount = m;
#endif
        return rdbuf.sgetn(s, m);
    }

    __istream_type& read(char_type* s, streamsize n)
    {
        // TODO: optimization point.  We want to do something
        // so that we don't inline this (and other read/write operations like it)
        // all over the place
        if (this->rdbuf()->sgetn(s, n) != n)
            // TODO: Consider setting _gcount here to what *was* returned
            this->setstate(base_type::eofbit);

#if FEATURE_ESTD_IOS_GCOUNT
        _gcount = n;
#endif

        return *this;
    }

    // TODO: optimize, ensure this isn't inlined
    __istream_type& getline(char_type* s, streamsize count, char_type delim = '\n')
    {
        streambuf_type* stream = this->rdbuf();

#if FEATURE_ESTD_IOS_GCOUNT
        _gcount = 0;
#endif

        for (;;)
        {
            int_type c = stream->sbumpc();

            if (traits_type::eq(c, traits_type::eof()))
            {
                this->setstate(base_type::eofbit);
                break;
            }

            if (!count-- || traits_type::eq(c, delim))
            {
                this->setstate(base_type::failbit);
                break;
            }

            *s++ = c;
#if FEATURE_ESTD_IOS_GCOUNT
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
        while (count--)
        {
            int_type ch = get();

            if (ch == traits_type::eof())
            {
                this->setstate(ios_base::eofbit);
                return *this;
            }

#if FEATURE_ESTD_IOS_GCOUNT
            _gcount++;
#endif
        }
        return *this;
    }

    // http://en.cppreference.com/w/cpp/io/basic_istream/ignore
    basic_istream& ignore(streamsize count, const int_type delim)
    {
        // "This [delimiter] test is disabled if delim is Traits::eof()"
        if (delim == traits_type::eof()) return ignore(count);

        while (count--)
        {
            int_type ch = get();

            if (ch == traits_type::eof())
            {
                this->setstate(ios_base::eofbit);
                break;
            }
            else if (ch == delim) break;

#if FEATURE_ESTD_IOS_GCOUNT
            _gcount++;
#endif
        }
        return *this;
    }


    basic_istream& sync()
    {
        if (this->rdbuf()->pubsync() == -1)
            this->setstate(base_type::badbit);

        return *this;
    }

    basic_istream& operator>>(basic_istream& (* __pf)(basic_istream&))
    {
        return __pf(*this);
    }


#ifndef FEATURE_IOS_STREAMBUF_FULL
    //typedef typename base_t::stream_type stream_t;

    //basic_istream(stream_t& stream) : base_t(stream) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    template <class ... TArgs>
    basic_istream(TArgs&& ...args) :
            base_type(std::forward<TArgs>(args)...)
    {
        gcount(0);
    }

    basic_istream(streambuf_type&& streambuf) :
            base_type(std::move(streambuf))
    {
        gcount(0);
    }
#else
    basic_istream()
    {
        gcount(0);
    }

    template<class T1>
    basic_istream(T1& param1) :
        base_type(param1)
    {
        gcount(0);
    }

#endif

    basic_istream(streambuf_type& streambuf) :
            base_type(streambuf)
    {
        gcount(0);
    }
#endif
};

} // detail

namespace internal {

template <class Streambuf, internal::istream_flags::flag_type flags = internal::istream_flags::_default>
using flagged_istream = detail::basic_istream<Streambuf,
            estd::internal::basic_ios<Streambuf, false,
                estd::internal::ios_base_policy<Streambuf, default_locale, flags> > >;

}}

#include "macro/pop.h"