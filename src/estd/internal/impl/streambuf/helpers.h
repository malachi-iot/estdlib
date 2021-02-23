#pragma once

#include "../streambuf.h"

namespace estd { namespace internal { namespace impl { namespace experimental {

template <class TStreambuf>
typename TStreambuf::int_type uflow(TStreambuf* sb)
{
    typedef typename TStreambuf::traits_type traits_type;
    typedef typename TStreambuf::int_type int_type;
    int_type ch = sb->underflow();

    if(ch != traits_type::eof())
        sb->gbump(1);

    return ch;
}

template <class TStreambuf>
typename TStreambuf::int_type seekoff(TStreambuf* sb)
{
    return 0;
}


template <class TStreambuf>
typename TStreambuf::int_type sputc(TStreambuf* sb, typename TStreambuf::int_type ch)
{
    return sb->overflow(ch);
}

template <class TStreambuf>
typename TStreambuf::pos_type seekoff_cur(TStreambuf* sb, int off, ios_base::openmode which)
{
    if(which & ios_base::in)
        sb->gbump(off);
    if(which & ios_base::out)
        sb->pbump(off);
}


}}}}
