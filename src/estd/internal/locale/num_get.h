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

template <class TChar, class InputIt = void, class TLocale = void>
class num_get
{
public:
    typedef TChar char_type;
    typedef InputIt iter_type;
    typedef TLocale locale_type;

private:

    struct _helper
    {
        // NOTE: iterated::num_get efficiently deals with unsigned as well
        // std::num_get deals with two different locales -- the one specified
        // on initial use_facet, then the one that is present in std::ios_base,
        // incoming by way of str.
        // Documentation [1] indicates to favor the latter
        template <unsigned base, class TIncomingLocale, class T>
        static iter_type get_signed_number(iter_type i, iter_type end,
            ios_base::iostate& err, TIncomingLocale l, T& v)
        {
            iterated::num_get<base, char_type, TIncomingLocale> n(l);
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


        template <unsigned base, class TIncomingLocale, class T>
        inline static iter_type get_signed_number(iter_type i, iter_type end,
            ios_base&, ios_base::iostate& err, TIncomingLocale l, T& v)
        {
            return get_signed_number<base>(i, end, err,l, v);
        }
    };

    template <class TStreambuf, class TBase>
    struct helper : _helper
    {
        typedef estd::detail::basic_istream<TStreambuf, TBase> istream_type;
        typedef typename istream_type::locale_type locale_type;

        template <unsigned base, class T>
        inline static iter_type get_signed_integer(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            return _helper::template get_signed_number<base>(
                i, end, str, err, str.getloc(),v);
        }

        template <unsigned base, class T>
        inline static iter_type get_unsigned_integer(iter_type i, iter_type end,
                                            ios_base::iostate& err, istream_type& str, T& v)
        {
            return _helper::template get_signed_number<base>(
                i, end, str, err, str.getloc(), v);
        }

        // For float and double
        template <class T>
        inline static iter_type get_float(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            return _helper::template get_signed_number<10>(
                i, end, str, err, str.getloc(), v);
        }

        template <bool boolalpha>
        static iter_type get_bool(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, bool& v)
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
            istream_type& str, ios_base::iostate& err,
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
            istream_type& str, ios_base::iostate& err,
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
            istream_type& str, ios_base::iostate& err,
            T& v,
            estd::true_type, estd::true_type)
        {
            const ios_base::fmtflags basefield = str.flags() & estd::ios_base::basefield;

#if FEATURE_ESTD_OSTREAM_OCTAL
            if(basefield == estd::ios_base::oct)
            {
                return get_signed_integer<8>(in, end, err, str, v);
            }
            else
#endif
            if(basefield == estd::ios_base::hex)
            {
                // No real negative in hex, so presume caller knows this and passed in a
                // signed type for their own convenience
                return get_unsigned_integer<16>(in, end, err, str, v);
            }
            else if(basefield == 0)
            {
                return get_interpreted(in, end, str, err, v);
            }
            else // if(basefield == estd::ios_base::dec)
            {
                return get_signed_integer<10>(in, end, err, str, v);
            }
        }

        // Floating point variety
        // types after 'v':
        // 'false_type' = is integer
        // 'true_type' = signed
        template <class T>
        static iter_type get(iter_type in, iter_type end,
            istream_type& str, ios_base::iostate& err,
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
            istream_type& str,ios_base::iostate& err,
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

    // NOTE: Keeping this private just so we don't offer too many non standard options
    template <unsigned base, typename T>
    inline iter_type get(iter_type in, iter_type end, ios_base::iostate& err, T& v) const
    {
        return _helper::template get_signed_number<base>(in, end, err, locale_type(), v);
    }


public:
    // NOTE: Non standard call.  locale is picked up from use_facet
    // DEBT: Non standard call.  At this time only supports base 8/10/16.
    // DEBT: Although convenient, I don't like adding confusion by deviating the spec this way.  In the end, it does
    // save a few bytes because ios_base doesn't need to exist and basefield can be optimized away.  Then again,
    // optimizers are so good they probably can optimize away ios_base as well.  May revert back to ios_base& only
    // flavor
    template <typename T>
#if __cplusplus >= 201402L
    [[deprecated("Unsure if we are keeping this API.  Use get(iter_type, iter_type, ios_base, ios_base::iostate, T) instead")]]
#endif
    inline iter_type get(iter_type in, iter_type end, const ios_base::fmtflags basefield, ios_base::iostate& err, T& v) const
    {
        switch(basefield)
        {
#if FEATURE_ESTD_OSTREAM_OCTAL
            case estd::ios_base::oct:
                return get<8>(in, end, err, v);
#endif

            case estd::ios_base::hex:
                return get<16>(in, end, err, v);

            default:
                return get<10>(in, end, err, v);
        }
    }

    // NOTE: Non standard call.  locale is picked up from use_facet
    template <typename T>
    iter_type get(iter_type in, iter_type end, const ios_base& str, ios_base::iostate& err, T& v) const
    {
        return get(in, end, str.flags() & estd::ios_base::basefield, err, v);
    }

    template <typename T, class TStreambuf, class TBase>
    iter_type get(iter_type in, iter_type end,
        estd::detail::basic_istream<TStreambuf, TBase>& str, ios_base::iostate& err, T& v) const
    {
        return helper<TStreambuf, TBase>::get(in, end, str, err, v,
           estd::is_integral<T>(), estd::is_signed<T>());
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