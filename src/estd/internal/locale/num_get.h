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

namespace estd {

namespace iterated {

// State machine flavor of num_get
template <unsigned base, typename TChar, class TLocale>
struct num_get
{
    //typedef estd::internal::basic_istream<TStreambuf, TBase> istream_type;
    //typedef typename istream_type::locale_type locale_type;
    typedef TLocale locale_type;
    typedef TChar char_type;
    typedef cbase<char_type, base, locale_type> cbase_type;
    //typedef ctype<char_type, locale_type> ctype_type;
    typedef typename cbase_type::optional_type optional_type;

    enum state
    {
        Start = 0,
        Header,
        FirstNegative,
        FirstPositive,
        NominalNegative,
        NominalPositive,
        Overflow,
        Complete
    };

    struct _state
    {
        state state_ : 4;
        //bool is_signed : 1;

        _state() : state_(Start) //, is_signed(false)
        {}

    } state_;

    // 'false_type' means this is the unsigned flavor
    template <bool positive, typename T>
    bool raise_and_add(optional_type n, T& v, false_type)
    {
        // Undefined/bad state.  Same as 'default' case switch
        // DEBT: Logging this and other internal failures would be nice.  In this case,
        // maybe we can do a static assert?
        if(!positive) return true;

        return estd::internal::raise_and_add(v, base, n.value());
    }

    // 'false_type' means this is the unsigned flavor
    template <bool positive, typename T>
    bool raise_and_add(optional_type n, T& v, true_type)
    {
        return positive ?
           estd::internal::raise_and_add(v, base, n.value()) :
           estd::internal::raise_and_sub(v, base, n.value());
    }

    template <bool positive, typename T>
    bool nominal(char_type c, ios_base::iostate& err, T& v)
    {
        optional_type n = cbase_type::from_char(c);

        if(n.has_value())
        {
            if (!raise_and_add<positive>(n, v, is_signed<T>()))
            {
                state_.state_ = Overflow;
                err |= ios_base::failbit;
            }

            return false;
        }

        return true;
    }

    // NOTE: This method never sets eof bit
    template <typename T>
    bool get(char_type c, ios_base::iostate& err, T& v)
    {
        switch(state_.state_)
        {
            case Start:
                v = 0;
                state_.state_ = NominalPositive;

                // DEBT: Revisit if we need to play with widening/narrowing/conversion
                // to ensure this hyphen compare is proper
                if (estd::is_signed<T>::value && c == '-')
                {
                    state_.state_ = NominalNegative;
                    return false;
                }

#if __has_cpp_attribute(fallthrough)
                [[fallthrough]];
#endif

            case FirstPositive:
                if(nominal<true>(c, err, v))
                {
                    // "if the conversion function fails std::ios_base::failbit is assigned to err" [1]
                    err |= ios_base::failbit;
                    return true;
                }
                else
                    return false;

            case FirstNegative:
                if(nominal<false>(c, err, v))
                {
                    // "if the conversion function fails std::ios_base::failbit is assigned to err" [1]
                    err |= ios_base::failbit;
                    return true;
                }
                else
                    return false;

            case NominalPositive:
                return nominal<true>(c, err, v);

            case NominalNegative:
                return nominal<false>(c, err, v);

            case Overflow:
                // Merely consumed numbers in this mode
                return cbase_type::from_char(c).has_value();

            // FIX: Just to satisfy compilation
            default:
                return true;
        }
    }


    template <class TIter, class T>
    bool get(TIter& i, TIter end,
        ios_base::iostate& err, T& v)
    {
        if(i == end)
        {
            err |= ios_base::eofbit;
            return true;
        }

        return get(*i++, err, v);
    }
};

}

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

        template <unsigned base, class T>
        static iter_type get_unsigned_integer(iter_type i, iter_type end,
                                             ios_base::iostate& err, istream_type& str, T& v)
        {
            iterated::num_get<base, char_type, locale_type> n;

            for(; i != end; ++i)
            {
                if(n.get(*i, err, v)) return i;
            }

            err |= ios_base::eofbit;
            return i;
        }


        // DEBT: We can likely place this elsewhere since it doesn't actually need 'this'
        //       though it being locale-bound may affect that down the line
        // NOTE: legacy version doesn't use state machine.  Due to our 'good' variable, I'd
        // rate our state machine as equally efficient as legacy version, but more powerful.
        // Given we can dogfood and reuse with apparently no penalty, we do it
        template <unsigned base, class T>
        static iter_type get_unsigned_integer_legacy(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            // DEBT: Consider using use_facet, though really not necessary at this time
            typedef cbase<char_type, base, locale_type> cbase_type;
            //use_facet<cbase<char_type, base> >(str.getloc()).from_char(*i);

            v = 0;
            // Since we are using forward-only iterators, we can't retain an
            // old 'i' to compare against
            bool good = false;

            for(; i != end; ++i, good = true)
            {
                typename cbase_type::optional_type n = cbase_type::from_char(*i);

                if(n.has_value())
                {
                    /**
                     * "If the conversion function fails to convert the entire field, the value 0
                     *  is stored in v
                     *
                     *  If the type of v is a signed integer type and the conversion
                     *  function results in a positive or negative value too large to fit in it,
                     *  the most positive or negative representable value is stored in v, respectively
                     *
                     *  If the type of v is an unsigned integer type and the conversion function results
                     *  in a value that does not fit in it, the most positive representable value is
                     *  stored in v." [1]
                     *
                     *  Beware that raise_and_add *might* clobber v in that condition.
                     *  See documentation for that function
                     *
                     *  "In any case, if the conversion function fails std::ios_base::failbit is
                     *   assigned to err" [1]
                     *
                     *  This *strongly implies* that the capped conversion is not considered a failure.
                     *  Something feels wrong about letting an overflow pass by unnoticed, so I am
                     *  setting the fail bit here despite what they imply
                     */
                    if(!estd::internal::raise_and_add(v, base, n.value()))
                    {
                        // TODO: It's subtle, but spec expects us to read all the digits out of
                        // the input iterator, even on overflow

                        err |= ios_base::failbit;
                        return i;
                    }
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

        template <unsigned base, class T>
        static iter_type get_signed_integer(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            bool negative = false;

            // DEBT: We could possibly constexpr up and down the facet/locale chain which may
            // more readily optimize this line.  That said, chances are it's already optimized
            // to the max (i.e. hypen = '-')
            // NOTE: spec strongly implies hyphen is *always* used to denote a negative number
            // regardless of locale
            const char hyphen =
                use_facet<ctype<char_type>>(str.getloc()).widen('-');

            // TODO: Might be able to merely copy this iterator and to this evaluation
            // at the end.  Perhaps do a specialization for this based on policy
            if(*i == hyphen)
            {
                negative = true;
                ++i;
            }

            i = get_unsigned_integer<base>(i, end, err, str, v);

            if(negative) v = -v;

            return i;
        }

        // DEBT: Works OK, but will get confused if names start with the same letters
        // DEBT: Move this out to a more utility oriented location, and give it a better name
        // Remember, we're limited how fancy we can get because we don't demand any kind of sorting
        // of the containers on the way in
        template <class TContainer, std::size_t N>
        static int chooser(const TContainer (&containers)[N], iter_type& in, iter_type end)
        {
            //bool good = false;

            int chosen = -1;

            for(int i = 0; in != end;
                ++in, ++i //, good = true
                )
            {
                char_type c = *in;

                // Look through all the containers to try to find the first match
                if(chosen == -1)
                {
                    for (unsigned j = 0; j < N; ++j)
                    {
                        const TContainer& container = containers[j];

                        if(container[i] == c)
                        {
                            chosen = j;
                            break;
                        }
                    }
                }
                else
                {
                    // DEBT: Consider doing this with a pointer instead
                    const TContainer& container = containers[chosen];

                    if(i == container.size())
                        return chosen;
                    else if(container[i] != c)
                        return -1;
                }
            }

            return chosen;
        }

        static iter_type get_bool(iter_type in, iter_type end,
            ios_base::iostate& err, istream_type& str, bool& v)
        {
            if(str.flags() & ios_base::boolalpha)
            {
                numpunct<char_type, locale_type> np =
                        use_facet<numpunct<char_type>>(str.getloc());

                // tempted to get algorithmically fancy here, but with only two things to
                // compare, brute force makes sense
                estd::layer2::basic_string<const char_type, 0>
                    names[] {
                    np.truename(),
                    np.falsename()
                };

                int chosen = chooser(names, in, end);

                if(in == end)
                    err |= ios_base::eofbit;

                if(chosen == -1)
                {
                    v = false;
                    err |= ios_base::failbit;
                }
                else
                    v = chosen == 0;
            }
            else
            {
                unsigned temp;
                in = get_unsigned_integer<2>(in, end, err, str, temp);
                // DEBT: Try to avoid using temporary.
                // No bounds check necessary here, since specifying base 2 already does that
                v = temp == 1;
            }
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

            if(basefield == estd::ios_base::oct)
            {
                return get_unsigned_integer<8>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
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

            if(basefield == estd::ios_base::oct)
            {
                return get_signed_integer<8>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
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

        // types after 'v':
        // 'true_type' = is integer
        // 'false_type' = unsigned
        static iter_type get(iter_type in, iter_type end,
            istream_type& str,ios_base::iostate& err,
            bool& v,
            estd::true_type, estd::false_type)
        {
            return get_bool(in, end, err, str, v);
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
        return helper<TStreambuf, TBase>::get(in, end, str, err, v,
           estd::is_integral<T>(), estd::is_signed<T>());
    }
};

namespace internal {

template <typename TChar, typename TInputIt, class TLocale>
struct use_facet_helper<num_get<TChar, TInputIt>, TLocale>
{
    typedef num_get<TChar, TInputIt> facet_type;

    static facet_type use_facet(TLocale) { return facet_type(); }
};

}

}