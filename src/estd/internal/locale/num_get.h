/**
 *
 * References
 *
 * 1. https://en.cppreference.com/w/cpp/locale/num_get/get
 */
#pragma once

#include "fwd.h"
#include "cbase.h"
#include "../ios_base.h"
#include "iterated/num_get.h"

#include "../../iosfwd.h"
#include "../streambuf.h"

namespace estd {

template <class Char, class InputIt = void, class Locale = void>
class num_get
{
public:
    using char_type = Char;
    using locale_type = Locale;

    typedef InputIt iter_type;

private:

    // DEBT: "helper" name sucks.  These guys help us adapt to incoming context either directly
    // specified or indirectly carried in via an istream
    struct localizer
    {
        // NOTE: iterated::num_get efficiently deals with unsigned as well
        // std::num_get deals with two different locales -- the one specified
        // on initial use_facet, then the one that is present in std::ios_base,
        // incoming by way of str.
        // Documentation [1] indicates to favor the latter
        template <unsigned base, class IncomingLocale, class T>
        static iter_type get_signed_number(iter_type i, iter_type end,
            ios_base::iostate& err, IncomingLocale l, T& v)
        {
            iterated::num_get<base, char_type, IncomingLocale> n(l);
            typedef bool_constant<numeric_limits<T>::is_integer> is_integer;
            // DEBT: We really ought to push these traits in from somewhere else
            typedef estd::iterator_traits<iter_type> iter_traits_type;
            typedef estd::char_traits<char_type> traits_type;

            // DEBT: iterated::num_get sometimes does this too
            v = 0;

            for(; i != end; ++i)
            {
                const typename iter_traits_type::value_type c = *i;

                if(traits_type::not_eof(c) == false)
                {
                    // TODO: Do some kind of poll/block here
                }

#if __cplusplus >= 201103L
                if(n.template get<false>(c, err, v))
#else
                if(n.get(c, err, v))
#endif
                {
                    n.finalize(v, is_integer());
                    return i;
                }
            }

            err |= ios_base::eofbit;
            n.finalize(v, is_integer());
            return i;
        }


        template <class IncomingLocale, typename T>
        static inline iter_type get(iter_type in, iter_type end,
            const ios_base::fmtflags basefield,
            ios_base::iostate& err,
            IncomingLocale l,
            T& v)
        {
            switch(basefield)
            {
#if FEATURE_ESTD_OSTREAM_OCTAL
                case estd::ios_base::oct:   return get_signed_number<8>(in, end, err,l, v);
#endif
                case estd::ios_base::hex:   return get_signed_number<16>(in, end, err,l, v);

                // DEBT: What about interpreted?
                default:
                    return get_signed_number<10>(in, end, err, l, v);
            }
        }
    };

    template <class Streambuf, class Base>
    struct istream_localizer : localizer
    {
        using base_type = localizer;
        using istream_type = detail::basic_istream<Streambuf, Base>;
        using locale_type = typename istream_type::locale_type;

        template <unsigned base, class T>
        inline static iter_type get_signed_integer(iter_type i, iter_type end,
            ios_base::iostate& err, const istream_type& str, T& v)
        {
            return base_type::template get_signed_number<base>(
                i, end, err, str.getloc(), v);
        }

        // NOTE: underlying iterated::num_get compile-time resolves whether we're signed or
        // unsigned, so this may be superfluous.
        template <unsigned base, class T>
        inline static iter_type get_unsigned_integer(iter_type i, iter_type end,
            ios_base::iostate& err, const istream_type& str, T& v)
        {
            return base_type::template get_signed_number<base>(
                i, end, err, str.getloc(), v);
        }

        // For float and double
        template <class T>
        inline static iter_type get_float(iter_type i, iter_type end,
            ios_base::iostate& err, const istream_type& str, T& v)
        {
            return base_type::template get_signed_number<10>(
                i, end, err, str.getloc(), v);
        }

        template <bool boolalpha>
        static iter_type get_bool(iter_type in, iter_type end,
            ios_base::iostate& err, const istream_type& str, bool& v)
        {
            estd::iterated::bool_get<char_type, locale_type, boolalpha> n(str.getloc());

            // DEBT: Some compilers want us to initialize v right away rather than in a state machine,
            // otherwise we get warnings.  Hopefully compilers are smart enough to figure out our pathing
            // in bool_get as we do always assign 'v'

            while(!n.get(in, end, err, v));

            return in;
        }

        // TODO: Need to build this out for 0x, 0, etc. support
        template <class T>
        static iter_type get_interpreted(iter_type in, iter_type end,
            const istream_type& str, ios_base::iostate& err,
            T& v)
        {
            err |= ios_base::failbit;
            return in;
        }


        // Special thanks to
        // https://stackoverflow.com/questions/9285657/sfinae-differentiation-between-signed-and-unsigned
        // for the hybrid overload/SFINAE approach below

        // unsigned int variety
        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            const istream_type& str, ios_base::iostate& err,
            T& v,
            estd::true_type, estd::false_type)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

            // TODO: Consider strongly using a switch statement, even though it does
            // spiritually break the spec "the first applicable choice of the following five is selected" [1]

#if FEATURE_ESTD_OSTREAM_OCTAL
            if(basefield == estd::ios_base::oct)
            {
                return get_unsigned_integer<8>(in, end, err, str, v);
            }
            else
#endif
            if(basefield == estd::ios_base::hex)
            {
                return get_unsigned_integer<16>(in, end, err, str, v);
            }
            else if(basefield == 0)
            {
                return get_interpreted(in, end, str, err, v);
            }
            else  //if(basefield == estd::ios_base::dec)
            {
                return get_unsigned_integer<10>(in, end, err, str, v);
            }
        }

        // signed int variety
        // types after 'v':
        // 'true_type' = is integer
        // 'true_type' = signed
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            const istream_type& str, ios_base::iostate& err,
            T& v,
            estd::true_type, estd::true_type)
        {
            const ios_base::fmtflags basefield = str.flags() & ios_base::basefield;

            switch(basefield)
            {
#if FEATURE_ESTD_OSTREAM_OCTAL
                case ios_base::oct: return get_signed_integer<8>(in, end, err, str, v);
#endif
                case ios_base::dec: return get_signed_integer<10>(in, end, err, str, v);
                case ios_base::hex: return get_signed_integer<16>(in, end, err, str, v);

                // 0 value
                default: return get_interpreted(in, end, str, err, v);
            }
        }

        // Floating point variety
        // types after 'v':
        // 'false_type' = is integer
        // 'true_type' = signed
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            const istream_type& str, ios_base::iostate& err,
            T& v,
            estd::false_type, estd::true_type)
        {
            return get_float(in, end, err, str, v);
        }

        // bool variety
        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        static iter_type get(iter_type in, iter_type end,
            const istream_type& str,ios_base::iostate& err,
            bool& v,
            estd::true_type, estd::false_type)
        {
            if(str.flags() & ios_base::boolalpha)
                return get_bool<true>(in, end, err, str, v);
            else
                return get_bool<false>(in, end, err, str, v);
        }
    };

    // TODO: Do a LUT since bounds checking to detect invalid hex chars likely is fastest.  See:
    // https://stackoverflow.com/questions/34365746/whats-the-fastest-way-to-convert-hex-to-integer-in-c
    // Also consider a cut-down one with only maybe 64 characters instead of 128 or 256, because unless
    // we do 256 we have to do bounds checking anyway


public:
    // NOTE: Non standard call.  locale is picked up from locale_type (use_facet)
    template <typename T>
    inline iter_type get(iter_type in, iter_type end, ios_base::fmtflags basefield, ios_base::iostate& err, T& v) const
    {
        return localizer::get(in, end, basefield, err, locale_type{}, v);
    }

    template <typename T, class Streambuf, class Base>
    iter_type get(iter_type in, iter_type end,
        const estd::detail::basic_istream<Streambuf, Base>& str, ios_base::iostate& err, T& v) const
    {
        return istream_localizer<Streambuf, Base>::get(in, end, str, err, v,
           is_integral<T>(), is_signed<T>());
    }
};

namespace internal {

template <typename TChar, typename TInputIt, class TLocale>
struct use_facet_helper<num_get<TChar, TInputIt>, TLocale>
{
    typedef num_get<TChar, TInputIt, TLocale> facet_type;

    static facet_type use_facet(TLocale) { return facet_type(); }
};

// Trick to convert a num_char<streambuf_type> into a
// num_char<char_type, istreambuf_iterator<streambuf_type> >
// DEBT: May be better doing this down at num_get level
// DEBT: Really would like this enable_if to take hold so that we could load in other
// types in addition to streambufs.  However, if all we can do is TStreambuf 'default' that
// is acceptable
template <typename TStreambuf
/*, typename estd::enable_if<
        estd::is_base_of<
                estd::internal::streambuf_baseline, TStreambuf>::value,
        TStreambuf>::type* = nullptr> */
        , class TLocale>
struct use_facet_helper<
    num_get<
            /*
        typename estd::enable_if<
            estd::is_base_of<
                estd::internal::streambuf_baseline, TStreambuf>::value,
            TStreambuf>::type, */
        TStreambuf,
        void >,
    TLocale>
    //internal::classic_locale_type>
{
    typedef TLocale locale_type;
    typedef num_get<
        typename TStreambuf::char_type,
        estd::istreambuf_iterator<TStreambuf>, locale_type > facet_type;
    //typedef internal::classic_locale_type locale_type;

    static facet_type use_facet(locale_type) { return facet_type(); }
};

}

}
