#pragma once

#include "../streambuf.h"
#include "tags.h"

// Helper area exists to:
// 1. declutter estd::streambuf
// 2. provide often-reused methods for impl::streambuf
// 3. offer specializations and other template tricks for use by estd::streambuf

namespace estd { namespace internal { namespace impl { namespace experimental {

struct streambuf_helper
{
    template <class TStreambuf>
    static inline typename TStreambuf::int_type sbumpc(TStreambuf* sb)
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
    static enable_if_t<!is_base_of<
            estd::experimental::streambuf_sbumpc_tag, TStreambuf>::value,
        typename TStreambuf::int_type>
    sbumpc_evaporated(TStreambuf* sb)
    {
        return sbumpc(sb);
    }

    template <class TStreambuf>
    static enable_if_t<is_base_of<
            estd::experimental::streambuf_sbumpc_tag, TStreambuf>::value,
        typename TStreambuf::int_type>
    sbumpc_evaporated(TStreambuf* sb)
    {
        typename TStreambuf::impl_type& sb_impl = *sb;
        return sb_impl.sbumpc();
    }
};




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
