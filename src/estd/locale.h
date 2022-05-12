#pragma once

#include "internal/platform.h"
#include "internal/locale.h"
#include "internal/iterator_standalone.h"
#include "internal/ios.h"
#include "internal/charconv.hpp"

#include "internal/iosfwd.h"

#include "string.h"

extern "C" {
#include <stdint.h>
}

namespace estd { namespace experimental {

template <class TChar>
struct numpunct;

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


template <locale_code_enum locale_code, internal::encodings::values encoding, class TChar>
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

template <locale_code::values locale_code, internal::encodings::values encoding>
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
    const char* name() const { return locale_name<locale_code, encoding>(); }
};



// specialization, deviating from standard in that locale is compile-time
// instead of runtime
// This has a number of implications, but mainly we are hard-wired
// to default-ASCII behaviors.  Ultimately this will be an issue but
// we can build out ctype at that time
// strongly implies a layer1 behavior
template <locale_code_enum locale_code>
class ctype<locale_code, estd::internal::encodings::ASCII, char> :
    public ctype_base,
    public locale<locale_code, estd::internal::encodings::ASCII>::facet
{
public:
    typedef char char_type;

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

template <locale_code_enum locale_code>
class ctype<locale_code, estd::internal::encodings::UTF8, char> :
    public ctype<locale_code, estd::internal::encodings::ASCII, char> {};


template <class TChar>
class ctype_test;


namespace layer5
{

template <class TChar>
class ctype : public ctype_base
{
public:
    virtual bool do_is(mask m, TChar c) const;
};


}

template <class TChar>
struct numpunct
{
    static estd::layer2::const_string truename() { return "true"; }
    static estd::layer2::const_string falsename() { return "false"; }
};

//template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
//bool has_facet(const locale<locale_code, encoding>& loc);

/*
template<locale_code_enum locale_code, internal::encodings::values encoding>
inline bool has_facet<ctype<locale_code, encoding, char> >(const locale<locale_code, encoding>&)
{
    return true;
}

template<>
inline const ctype<char>& use_facet(const locale&)
{
    static ctype<char> facet;

    return facet;
} */

// FIX: Just to get things compiling, hardcoding these
template <class TFacet, class TLocale>
inline bool has_facet(const TLocale&) { return true; }



template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
inline TFacet use_facet(const locale<locale_code, encoding>&) { return TFacet(); }
//template <class TFacet, class TLocale>
//inline TFacet use_facet(TLocale) { return TFacet(); }

/*
 * TODO: Try use_facet_ctype, which is not standard, but may be the best we can do without going
 * full trailing/auto return type
template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet2<ctype_test<TChar>>(const locale<locale_code, encoding>&)
{
    return ctype<locale_code, encoding, TChar>();
} */

template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet_ctype(const locale<locale_code, encoding>&)
{
    return ctype<locale_code, encoding, TChar>();
}

template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
struct use_facet_helper<ctype_test<TChar>, locale_code, encoding>
{
    typedef ctype<locale_code, encoding, TChar> value_type;

    static value_type use_facet(const locale<locale_code, encoding>&)
    {
        return value_type();
    }
};

template <class TFacet, locale_code_enum locale_code, internal::encodings::values encoding>
inline typename use_facet_helper<TFacet, locale_code, encoding>::value_type
    use_facet3(const locale<locale_code, encoding>& loc)
{
    typedef use_facet_helper<TFacet, locale_code, encoding> helper_type;

    return helper_type::use_facet(loc);
}

/*
template <class TChar, locale_code_enum locale_code, internal::encodings::values encoding>
inline ctype<locale_code, encoding, TChar> use_facet3(const locale<locale_code, encoding>& loc)
{
    //typedef use_facet_helper<ctype_test<TChar>> helper_type;

    //helper_type::use_facet(loc);
    return ctype<locale_code, encoding, TChar>();
} */


template <locale_code_enum locale_code, internal::encodings::values encoding, class TChar>
inline bool isspace(TChar ch, const locale<locale_code, encoding>& loc)
{
    typedef ctype<locale_code, encoding, TChar> ctype_type;
    return use_facet<ctype_type>(loc).is(ctype_base::space, ch);
}


// DEBT: Consolidate all this with char_base_traits
template <class TChar, class InputIt>
class num_get
{
public:
    typedef TChar char_type;
    typedef InputIt iter_type;

private:

    template <class TStreambuf, class TBase>
    struct helper
    {
        typedef estd::internal::basic_istream<TStreambuf, TBase> istream_type;
        typedef typename istream_type::locale_type locale_type;

        // DEBT: We can likely place this elsewhere since it doesn't actually need 'this'
        //       though it being locale-bound may affect that down the line
        template <unsigned base, class T>
        static iter_type get_unsigned_integer_ascii(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            typedef estd::internal::char_base_traits<base> char_base_traits;
            typedef typename char_base_traits::int_type int_type;

            v = 0;
            // Since we are using forward-only iterators, we can't retain an
            // old 'i' to compare against
            bool good = false;

            for(; i != end; ++i, good = true)
            {
                int_type n = char_base_traits::from_char_with_test(*i);

                if(n != char_base_traits::eol())
                {
                    estd::internal::raise_and_add(v, char_base_traits::base(), n);
                }
                else
                {
                    if(!good)
                        err |= ios_base::failbit;

                    return i;
                }
            }

            err |= ios_base::eofbit;
            return i;
        }

        template <class T>
        static iter_type get_signed_integer_ascii_decimal(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            bool negative = false;

            // TODO: Might be able to merely copy this iterator and to this evaluation
            // at the end.  Perhaps do a specialization for this based on policy
            if(*i == '-')
            {
                negative = true;
                ++i;
            }

            i = get_unsigned_integer_ascii<10>(i, end, err, str, v);

            if(negative) v = -v;

            return i;
        }

        static iter_type get_bool_ascii(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, bool& v)
        {
            if(str.flags() & ios_base::boolalpha)
            {
                locale_type locale = str.getloc();
                numpunct<char> np;

                // tempted to get algorithmically fancy here, but with only two things to
                // compare, brute force makes sense
                estd::layer2::const_string names[] {
                    np.truename(),
                    np.falsename()
                };

                bool good = false;
                int chosen = -1;
                v = false;

                // DEBT: const_iterator is broken here, we can't increment/decrement it
                estd::layer2::const_string::iterator
                    true_it = np.truename().begin(),
                    true_end = np.truename().end(),
                    false_it = np.falsename().begin(),
                    false_end = np.falsename().end();

                for(int i = 0; in != end;
                    ++in, ++i, ++true_it, ++false_it, good = true)
                {
                    char_type c = *in;

                    if(*true_it == c)
                    {
                        if(chosen == 1)
                        {
                            err |= ios_base::failbit;
                            break;
                        }
                        // 'true' character detected
                        chosen = 0;
                        if(i == names[0].size() - 1)
                        {
                            v = true;
                            break;
                        }
                    }
                    else if(*false_it == c)
                    {
                        if(chosen == 0)
                        {
                            err |= ios_base::failbit;
                            break;
                        }
                        // 'false' character detected
                        chosen = 1;
                        if(i == names[1].size() - 1)
                        {
                            break;
                        }
                    }
                    else
                    {
                        err |= ios_base::failbit;
                        break;
                    }
                }
            }
            else
            {
                unsigned temp;
                in = get_unsigned_integer_ascii<2>(in, end, err, str, temp);
                // DEBT: Try to avoid using temporary.
                // No bounds check necessary here, since specifying base 2 already does that
                v = temp == 1;
            }
            return in;
        }

        template <typename T>
        static iter_type get_integer_ascii(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

            if(basefield == estd::ios_base::dec)
            {
                return get_signed_integer_ascii_decimal(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                // No real negative in hex, so presume caller knows this and passed in a
                // signed type for their own convenience
                return get_unsigned_integer_ascii<16>(in, end, err, str, v);
            }

            return in;
        }


        template <typename T>
        static iter_type get_unsigned_ascii(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

            if(basefield == estd::ios_base::dec)
            {
                get_unsigned_integer_ascii<10>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                get_unsigned_integer_ascii<16>(in, end, err, str, v);
            }

            return in;
        }

        // Special thanks to
        // https://stackoverflow.com/questions/9285657/sfinae-differentiation-between-signed-and-unsigned
        // for the hybrid overload/SFINAE approach below

        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str,
            T& v,
            estd::true_type, estd::false_type)
        {
            return get_unsigned_ascii(in, end, err, str, v);
        }

        // types after 'v':
        // 'true_type' = is integer
        // 'true_type' = signed
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str,
            T& v,
            estd::true_type, estd::true_type)
        {
            return get_integer_ascii(in, end, err, str, v);
        }

        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        static iter_type get(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str,
            bool& v,
            estd::true_type, estd::false_type)
        {
            return get_bool_ascii(in, end, err, str, v);
        }
    };

    // TODO: Do a LUT since bounds checking to detect invalid hex chars likely is fastest.  See:
    // https://stackoverflow.com/questions/34365746/whats-the-fastest-way-to-convert-hex-to-integer-in-c
    // Also consider a cut-down one with only maybe 64 characters instead of 128 or 256, because unless
    // we do 256 we have to do bounds checking anyway


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




public:
    template <typename T, class TStreambuf, class TBase>
    iter_type get(iter_type in, iter_type end,
        estd::internal::basic_istream<TStreambuf, TBase>& str, ios_base::iostate& err, T& v) const
    {
        return helper<TStreambuf, TBase>::get(in, end, err, str, v,
           estd::is_integral<T>(), estd::is_signed<T>());
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
