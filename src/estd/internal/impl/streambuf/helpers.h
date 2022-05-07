#pragma once

#include "../streambuf.h"
#include "tags.h"

namespace estd { namespace internal { namespace impl { namespace experimental {

template <class TStreambuf>
inline typename TStreambuf::int_type sbumpc(TStreambuf* sb)
{
    typedef typename TStreambuf::traits_type traits_type;
    typedef typename TStreambuf::int_type int_type;
    int_type ch = sb->sgetc();

    if(ch != traits_type::eof())
    {
        sb->gbump(1);
        return ch;
    }
    else
        return sb->uflow();
}

template <class TStreambuf>
enable_if_t<!is_base_of<
    estd::experimental::streambuf_sbumpc_tag, TStreambuf
>::value, typename TStreambuf::int_type>
sbumpc_evaporated(TStreambuf* sb)
{
    return sb->sbumpc_legacy();
}

template <class TStreambuf>
enable_if_t<is_base_of<
    estd::experimental::streambuf_sbumpc_tag, TStreambuf
>::value, typename TStreambuf::int_type>
sbumpc_evaporated(TStreambuf* sb)
{
    return ((typename TStreambuf::impl_type*)sb)->sbumpc();
}


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
