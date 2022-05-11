/**
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/io/ios_base/iostate
 */
#pragma once

#include "runtime_array.h"
#include "istream.h"
#include "iosfwd.h"

namespace estd {

namespace internal {

template <class TStreambuf, class TBase, class TStringImpl>
void do_input(internal::basic_istream<TStreambuf, TBase>& in,
        internal::dynamic_array<TStringImpl>& value)
{
    typedef typename internal::basic_istream<TStreambuf, TBase> istream_type;
    typedef typename estd::remove_reference<TStreambuf>::type impl_type;
    typedef typename impl_type::traits_type traits_type;
    //typedef typename impl_type::char_type char_type;
    typedef typename impl_type::int_type int_type;
    typedef typename istream_type::locale_type locale_type;
    typedef typename istream_type::policy_type policy_type;
    typedef typename istream_type::blocking_type blocking_type;

    locale_type loc = in.getloc();

    for(;;)
    {
        int_type ch = in.peek();

        if(ch == traits_type::eof())
        {
            // If we're non blocking variety, and rdbuf says "unsure if more characters
            // are available", then do our special nodata processing
            if((policy_type::blocking() == false) && in.rdbuf()->in_avail() == 0)
            {
                blocking_type::on_nodata(in, in.rdbuf(), value.size());
            }
            else
            {
                if(value.empty())
                    // "if the function extracts no characters from the input stream." [1]
                    in.setstate(istream_type::failbit);

                in.setstate(istream_type::eofbit);
            }

            break;
        }
        else if(isspace(traits_type::to_char_type(ch), loc)) break;

        // NOTE: += is defined and should have worked
        value.push_back(traits_type::to_char_type(ch));
        //value += (char_type)ch;

        in.get();
    }
}

// Since using block_type policy at ios level, peek now blocks in that context
// so regular input code works
template <class TStreambuf, class TBase, class TStringImpl>
void blocking_input_helper(internal::basic_istream<TStreambuf, TBase>& in,
        internal::dynamic_array<TStringImpl>& value)
{
    typedef typename internal::basic_istream<TStreambuf, TBase> istream_type;
    typedef typename estd::remove_reference<TStreambuf>::type impl_type;
    typedef typename impl_type::traits_type traits_type;
    typedef typename impl_type::char_type char_type;
    typedef typename impl_type::int_type int_type;
    typedef typename istream_type::streambuf_type streambuf_type;
    typedef typename istream_type::locale_type locale_type;

    locale_type loc = in.getloc();
    streambuf_type* rdbuf = in.rdbuf();

    for(;;)
    {
        int_type ch;

        // FIX: Was a bit of confusion because of seemingly redundant xin_avail
        // and showmanyc
        while(rdbuf->in_avail() == 0) // && rdbuf->sgetc() == -1)
        {

        }

        ch = rdbuf->sbumpc();
        
        // NOTE: If we enable this line, we get some kind of program fault.
        // That's because we never get an eof or space, and value.push_back
        // overflows
        //ch = 'h';

        if(ch == traits_type::eof())
        {
            in.setstate(istream_type::failbit | istream_type::eofbit);
            break;
        }
        else if(isspace((char_type)ch, loc)) break;

        //*dest++ = ch;

        // NOTE: += is defined and should have worked
        value.push_back((char_type)ch);
    }
}

}


// NOTE: This will work but doesn't filter specifically by string, which perhaps we want
template <class TStreambuf, class TBase, class TStringImpl>
internal::basic_istream<TStreambuf, TBase>& operator >>(
        internal::basic_istream<TStreambuf, TBase>& in,
        internal::dynamic_array<TStringImpl>& value)
{
    in >> ws;

    value.clear();

    internal::do_input(in, value);

    return in;
}


// NOTE: Experimental, only use if above doesn't satisfy things
/*
template <class TImpl, class TStringAllocator, class TStringPolicy>
internal::basic_istream<TImpl>& operator >>(internal::basic_istream<TImpl>& in,
                                            basic_string<
                                                typename TImpl::char_type,
                                                typename TImpl::traits_type,
                                                TStringAllocator,
                                                TStringPolicy>&
                                            value)
{
    return in;
}
*/

/*
 * Up against this error:

Undefined symbols for architecture x86_64:

"estd::internal::basic_istream<
    estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::internal::basic_ios<estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&>
>& estd::operator>><
    estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::layer1::basic_string<
        char, 32ul, true, std::__1::char_traits<char>,
        estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
    >
>(
    estd::internal::basic_istream<estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::internal::basic_ios<
        estd::internal::streambuf<
            estd::internal::impl::basic_stringbuf<
                estd::layer1::basic_string<
                    char, 32ul, true, std::__1::char_traits<char>,
                    estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
                >
            >
        >&>
    >&,
    estd::layer1::basic_string<
        char, 32ul, true, std::__1::char_traits<char>,
        estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false> >&
)", referenced from:
      ____C_A_T_C_H____T_E_S_T____0() in ios-test.cpp.o

 */
/*
// NOTE: This isn't active yet.  Specialization still broken for prototype in istream.h
// pretty sure it has to do with us not getting the specialization signature *exactly right*
// so that when the compiler chooses the definition based on the prototype it matches up
// with the implementation here
template <class TImpl, class TBase, unsigned N, class CharT, class Traits, class Policy>
internal::basic_istream<TImpl, TBase>& operator >>(
        internal::basic_istream<TImpl, TBase>& in,
        layer1::basic_string<
            //typename TImpl::char_type,
            CharT,
            N, true,
            //typename TImpl::traits_type
            Traits,
            Policy
            >& s)
{
    return in;
}
*/
}
