#pragma once

#include "fwd.h"
#include "cbase.h"

namespace estd { namespace experimental {

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
        static iter_type get_unsigned_integer(iter_type i, iter_type end,
            ios_base::iostate& err, istream_type& str, T& v)
        {
            // DEBT: Consider using use_facet, though really not necessary at this time
            typedef cbase<char_type, base, locale_type> cbase_type;
            //use_facet4<cbase<char_type, base> >(str.getloc()).from_char(*i);

            v = 0;
            // Since we are using forward-only iterators, we can't retain an
            // old 'i' to compare against
            bool good = false;

            for(; i != end; ++i, good = true)
            {
                typename cbase_type::optional_type n = cbase_type::from_char(*i);

                if(n.has_value())
                {
                    estd::internal::raise_and_add(v, base, *n);
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
                use_facet4<ctype<char_type>>(str.getloc()).widen('-');

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
                        use_facet4<numpunct<char_type>>(str.getloc());

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

            if(basefield == estd::ios_base::oct)
            {
                get_unsigned_integer<8>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::dec)
            {
                get_unsigned_integer<10>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                get_unsigned_integer<16>(in, end, err, str, v);
            }

            return in;
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
            else if(basefield == estd::ios_base::dec)
            {
                return get_signed_integer<10>(in, end, err, str, v);
            }
            else if(basefield == estd::ios_base::hex)
            {
                // No real negative in hex, so presume caller knows this and passed in a
                // signed type for their own convenience
                return get_unsigned_integer<16>(in, end, err, str, v);
            }

            return in;
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


template <typename TChar, typename TInputIt, class TLocale>
struct use_facet_helper4<num_get<TChar, TInputIt>, TLocale>
{
    typedef num_get<TChar, TInputIt> facet_type;

    static facet_type use_facet(TLocale) { return facet_type(); }
};

}}