#pragma once

#include "internal/platform.h"
#include "internal/locale.h"
#include "iterator.h"
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

template <class TChar, class InputIt>
class num_get
{
public:
    typedef TChar char_type;
    typedef InputIt iter_type;

    template <typename T>
    iter_type get(iter_type in, iter_type end,
        estd::ios_base::iostate& err,
        estd::ios_base& str,
        T& v) const;

    iter_type get(iter_type in, iter_type end,
        estd::ios_base::iostate& err, estd::ios_base& str, long& v) const
    {
        return in;
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
