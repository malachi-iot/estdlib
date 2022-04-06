#pragma once

#include "internal/platform.h"
#include "internal/locale.h"
#include "internal/iterator_standalone.h"
#include "internal/ios.h"

extern "C" {
#include <stdint.h>
}

namespace estd { namespace experimental {

struct ctype_base
{
    typedef uint8_t mask;

    static CONSTEXPR mask space = 0x01;
    static CONSTEXPR mask digit = 0x02;
    static CONSTEXPR mask alpha = 0x04;
    static CONSTEXPR mask punct = 0x08;
    static CONSTEXPR mask upper = 0x10;
    static CONSTEXPR mask lower = 0x20;
    static CONSTEXPR mask xdigit = 0x40;

    static CONSTEXPR mask alnum = alpha | digit;
    static CONSTEXPR mask graph = alnum | punct;
};


template <class TChar>
class ctype : public ctype_base
{
#ifdef ENABLE_LOCALE_MULTI
    // TODO: determine if we want to roll with the virtual function do_is
    // and friends or branch out into further templating
#else
    TChar do_tolower(TChar ch);
    TChar do_toupper(TChar ch);
#endif
public:
    bool is(mask m, TChar ch) const { return false; }
    const TChar* is(const TChar* low, const TChar* high, mask* vec) const { return NULLPTR; }

    TChar toupper(TChar ch) { return do_toupper(ch); }
    TChar tolower(TChar ch) { return do_tolower(ch); }
};

struct locale
{
#ifdef ENABLE_LOCALE_MULTI
    struct facet
    {

    };

    typedef int id;

    // FIX: 40 arbitrary number, could be more or less
    // NOTE: seems kind of like a fake-rtti system a bit
    facet* facets[40];

    locale(const locale& other);
    explicit locale(const char* std_name);
#else
    struct facet
    {

    };

    struct id
    {

    };
#endif

    typedef int category;

    static CONSTEXPR category none = 0x0000;
    static CONSTEXPR category ctype = 0x0001;
    static CONSTEXPR category numeric = 0x0002;

    // TODO: deviates in that standard version uses a std::string
    // I want my own std::string (beginnings of which are in experimental::layer3::string)
    // but does memory allocation out of our own GC-pool
    const char* name() const { return "*"; }
};



// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <>
class ctype<char> : public ctype_base, public locale::facet
{
public:
    //static locale::id id;

    char widen(char c) const { return c; }

    bool is(mask m, char ch) const
    {
        if(m & space)
        {
            // as per http://en.cppreference.com/w/cpp/string/byte/isspace
            switch(ch)
            {
                case ' ':
                case 13:
                case 10:
                case '\f':
                case '\t':
                case '\v':
                    return true;
            }
        }
        if(m & digit)
        {
            if(ch >= '0' && ch <= '9') return true;
        }
        return false;
    }
};


namespace layer5
{

template <class TChar>
class ctype : public ctype_base
{
public:
    virtual bool do_is(mask m, TChar c) const;
};


}

template <class TFacet>
bool has_facet(const locale& loc);

template<>
inline bool has_facet<ctype<char> >(const locale&)
{
    return true;
}

template<>
inline const ctype<char>& use_facet(const locale&)
{
    static ctype<char> facet;

    return facet;
}

template <class TChar>
inline bool isspace(TChar ch, const locale& loc)
{
    return use_facet<ctype<TChar> >(loc).is(ctype_base::space, ch);
}


// DEBT: Consolidate all this with char_base_traits
template <class TChar, class InputIt>
class num_get
{
public:
    typedef TChar char_type;
    typedef InputIt iter_type;

private:

    // TODO: Do a LUT since bounds checking to detect invalid hex chars likely is fastest.  See:
    // https://stackoverflow.com/questions/34365746/whats-the-fastest-way-to-convert-hex-to-integer-in-c
    // Also consider a cut-down one with only maybe 64 characters instead of 128 or 256, because unless
    // we do 256 we have to do bounds checking anyway


    template <class T>
    void get_integer_ascii_hexadecimal(iter_type i, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        v = 0;

        for(; i < (end - 1); ++i)
        {
            char_type c = *i;

            v <<= 4;

            if(c >= '0' && c <= '9')
            {
                v += c - '0';
            }
            else if(c >= 'A' && c <= 'F')
            {
                v += 10;
                v += c - 'A';
            }
            else if(c >= 'a' && c <= 'f')
            {
                v += 10;
                v += c - 'a';
            }
            else
            {
                // DEBT: We may not have to do this, 'v' may be acceptable as undefined
                // if we reach failbit
                v >>= 4;

                err |= ios_base::failbit;
                return;
            }
        }

        err |= ios_base::eofbit;
    }

    // Lifted from
    // https://stackoverflow.com/questions/221001/performance-question-fastest-way-to-convert-hexadecimal-char-to-its-number-valu
    template <class T>
    void get_integer_ascii_hexadecimal_unchecked(iter_type i, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        v = 0;

        // Doesn't detect errors

        for(; i < (end - 1); ++i)
        {
            char_type c = *i;

            c|=0x20;

            c = c<='9'? c+0xD0 : c+0xA9;

            v <<= 4;
            v += c;
        }

        err |= ios_base::eofbit;
    }

    template <class T>
    void get_unsigned_integer_ascii_decimal(iter_type i, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        v = 0;

        for(; i < (end - 1); ++i)
        {
            const char_type c = *i;
            // DEBT: Need to cleverly use a signed integer whose width matches char_type
            const int16_t b = c - '0';

            if(b >= 0 && b <= 9)
            {
                v *= 10;
                v += b;
            }
            else
            {
                err |= ios_base::failbit;
                return;
            }
        }

        err |= ios_base::eofbit;
    }

    template <class T>
    void get_signed_integer_ascii_decimal(iter_type i, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        bool negative = false;

        // TODO: Might be able to merely copy this iterator and to this evaluation
        // at the end.  Perhaps do a specialization for this based on policy
        if(*i == '-')
        {
            negative = true;
            ++i;
        }

        get_unsigned_integer_ascii_decimal(i, end, err, str, v);

        if(negative) v = -v;
    }

public:
    template <typename T>
    iter_type get(iter_type in, iter_type end,
        estd::ios_base::iostate& err,
        estd::ios_base& str,
        T& v) const;

    template <typename T>
    iter_type get_integer_ascii(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

        if(basefield == estd::ios_base::dec)
        {
            get_signed_integer_ascii_decimal(in, end, err, str, v);
        }
        else if(basefield == estd::ios_base::hex)
        {
            get_integer_ascii_hexadecimal(in, end, err, str, v);
        }

        return in;
    }


    template <typename T>
    iter_type get_unsigned_ascii(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, T& v) const
    {
        const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

        if(basefield == estd::ios_base::dec)
        {
            get_unsigned_integer_ascii_decimal(in, end, err, str, v);
        }
        else if(basefield == estd::ios_base::hex)
        {
            get_integer_ascii_hexadecimal(in, end, err, str, v);
        }

        return in;
    }

    iter_type get(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, unsigned& v) const
    {
        return get_integer_ascii(in, end, err, str, v);
    }


    iter_type get(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, unsigned short& v) const
    {
        return get_integer_ascii(in, end, err, str, v);
    }

    iter_type get(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, short& v) const
    {
        return get_integer_ascii(in, end, err, str, v);
    }



    iter_type get(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, int& v) const
    {
        return get_integer_ascii(in, end, err, str, v);
    }

    iter_type get(iter_type in, iter_type end,
        ios_base::iostate& err, ios_base& str, long& v) const
    {
        return get_integer_ascii(in, end, err, str, v);
    }
};

/*
 * Can't remember if this is possible / how to do it
template <class TChar, class InputIt>
template <>
inline typename num_get<TChar, InputIt>::iter_type num_get<TChar, InputIt>::get<long>(
    iter_type in,
    iter_type end, estd::ios_base::iostate& err, long& v) const
{
    return in;
}
 */

}}
