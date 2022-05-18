/***
 * State machine flavor of num_get
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/locale/num_get/get
 */
#pragma once

#include "../cbase.h"
#include "../fwd.h"
#include "../../ios_base.h"
#include "../../chooser.h"

namespace estd { namespace iterated {

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
    typedef typename cbase_type::int_type int_type;

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
    inline static bool raise_and_add(int_type n, T& v, false_type)
    {
        // Undefined/bad state.  Same as 'default' case switch
        // DEBT: Logging this and other internal failures would be nice.  In this case,
        // maybe we can do a static assert?
        if(!positive) return true;

        return estd::internal::raise_and_add(v, base, n);
    }

    // 'true_type' means this is the signed flavor
    template <bool positive, typename T>
    inline static bool raise_and_add(int_type n, T& v, true_type)
    {
        return positive ?
           estd::internal::raise_and_add(v, base, n) :
           estd::internal::raise_and_sub(v, base, n);
    }

    template <bool positive, typename T>
    bool nominal(char_type c, ios_base::iostate& err, T& v)
    {
        optional_type n = cbase_type::from_char(c);

        if(n.has_value())
        {
            if (!raise_and_add<positive>(n.value(), v, is_signed<T>()))
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

    // Just a bit of future proofing
    num_get(TLocale) {}
    num_get() {}
};


template <typename TChar, class TLocale>
struct num_get<0, TChar, TLocale>
{
    union
    {
        num_get<8, TChar, TLocale> base8;
        num_get<10, TChar, TLocale> base10;
        num_get<16, TChar, TLocale> base16;
    };
};



}}