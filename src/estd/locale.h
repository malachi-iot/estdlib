#pragma once

extern "C" {
#include <stdint.h>
}

namespace FactUtilEmbedded { namespace std { namespace experimental {

struct ctype_base
{
    typedef uint8_t mask;

    static constexpr mask space = 0x01;
    static constexpr mask digit = 0x02;
    static constexpr mask alpha = 0x04;
    static constexpr mask punct = 0x08;

    static constexpr mask alnum = alpha | digit;
    static constexpr mask graph = alnum | punct;
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
    const TChar* is(const TChar* low, const TChar* high, mask* vec) const { return nullptr; }

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

    static constexpr category none = 0x0000;
    static constexpr category ctype = 0x0001;
    static constexpr category numeric = 0x0002;

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
    static locale::id id;

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
const TFacet& use_facet(const locale& loc);

template <class TFacet>
bool has_facet(const locale& loc);

template<>
inline bool has_facet<ctype<char>>(const locale& loc)
{
    return true;
}

template<>
inline const ctype<char>& use_facet(const locale& loc)
{
    static ctype<char> facet;

    return facet;
}

template <class TChar>
inline bool isspace(TChar ch, const locale& loc)
{
    return use_facet<ctype<TChar>>(loc).is(ctype_base::space, ch);
}

}}}