/***
 * State machine flavor of num_get
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/locale/num_get/get
 *
 * Notes:
 *
 * "If Stage 2 was terminated by the test in==end, err|=std::ios_base::eofbit
 *  is executed to set the eof bit."
 *
 * This is somewhat nuanced.  This means that certain types such as integer, we plunge
 * forward until delimiter or EOF.  bool, however, we know where the end is - so EOF
 * happens less often with a bool.
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
    // NOTE: Doing autoinit because compilers sometimes don't trust that we really do initialize
    // v, causing "maybe-uninitialized" warning.  To handle this, external parties may elect to
    // init to zero instead of us.
    // DEBT: Guard against availability of autoinit with something more cohesive than just __cplusplus
    template <
#if __cplusplus >= 201103L
        bool autoinit = true,
#endif
        typename T>
    bool get(char_type c, ios_base::iostate& err, T& v)
    {
        switch(state_.state_)
        {
            case Start:
#if __cplusplus >= 201103L
                if(autoinit) v = 0;
#else
                v = 0;
#endif
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
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
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

template <typename TChar, class TLocale, bool boolalpha>
struct bool_get;

// numeric (non alpha) version
template <typename TChar, class TLocale>
struct bool_get<TChar, TLocale, false> : num_get<2, TChar, TLocale>
{
    typedef TChar char_type;
    //typedef num_get<2, char_type, TLocale> base_type;

    bool get(char_type c, ios_base::iostate& err, bool& v)
    {
        switch(c)
        {
            case '0':
                v = false;
                break;

            default:
                err |= ios_base::failbit;
#if __has_cpp_attribute(fallthrough)
                [[fallthrough]];
#endif

            case '1':
                v = true;
                break;
        }

        // Numeric bool is always one character, so we're always done no matter
        // what the input
        return true;
    }

    template <class TIter>
    bool get(TIter& i, TIter end, ios_base::iostate& err, bool& v)
    {
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
    }


    bool_get(TLocale l)
    //    : base_type(l)
    {}
    bool_get() {}
};

// alpha version
template <typename TChar, class TLocale>
struct bool_get<TChar, TLocale, true>
{
    typedef TChar char_type;
    typedef TLocale locale_type;
    typedef estd::numpunct<char_type, locale_type> numpunct_type;

    enum state
    {
        Start = 0
    };

    internal::chooser chooser;
    estd::layer2::basic_string<const char_type, 0> names[2];

    // NOTE: This method never sets eof bit
    bool get(char_type c, ios_base::iostate& err, bool& v)
    {
        if(chooser.process(names, c) == false)
            return false;

        if(chooser.chosen() != -1)
        {
            v = chooser.chosen() == 0;
        }
        else
        {
            v = false;
            err |= ios_base::failbit;
        }

        return true;
    }

    template <class TIter>
    bool get(TIter& i, TIter end, ios_base::iostate& err, bool& v)
    {
        if(i != end) return get(*i++, err, v);

        err |= ios_base::eofbit;
        return true;
    }


    void set_names(locale_type l)
    {
        estd::numpunct<char_type, locale_type> n = use_facet<numpunct<char> >(l);
        names[0] = n.truename();
        names[1] = n.falsename();
    }

#if __cplusplus >= 201103L
    // DEBT: Not instance-locale compat
    bool_get(locale_type l) : names { numpunct_type::truename(), numpunct_type::falsename() }
    {}
    bool_get() : names { numpunct_type::truename(), numpunct_type::falsename() }
    {}
#else
    bool_get() { set_names(locale_type()); }
    bool_get(locale_type l) { set_names(l); }
#endif
    // { set_names(); }
};



}}