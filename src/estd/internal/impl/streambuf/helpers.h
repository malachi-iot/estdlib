#pragma once

#include "../streambuf.h"
#include "tags.h"

// Helper area exists to:
// 1. declutter estd::streambuf
// 2. provide often-reused methods for impl::streambuf
// 3. offer specializations and other template tricks for use by estd::streambuf

namespace estd { namespace internal { namespace impl { 

// DEBT: Lousy name
struct streambuf_helper
{
    // Default sungetc which relies on gptr and eback
    template <class TStreambuf>
    static inline typename TStreambuf::int_type sungetc_full(TStreambuf* sb)
    {
        typedef typename TStreambuf::traits_type traits_type;

        if(sb->gptr() > sb->eback())
        {
            sb->gbump(-1);
            return traits_type::to_int_type(*sb->gptr());
        }

        return sb->pbackfail();
    }

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
    static typename enable_if<!is_base_of<
            streambuf_sbumpc_tag, TStreambuf>::value,
        typename TStreambuf::int_type>::type
    sbumpc_evaporated(TStreambuf* sb)
    {
        return sbumpc(sb);
    }

    template <class TStreambuf>
    static typename enable_if<is_base_of<
            streambuf_sbumpc_tag, TStreambuf>::value,
        typename TStreambuf::int_type>::type
    sbumpc_evaporated(TStreambuf* sb)
    {
        typename TStreambuf::impl_type& sb_impl = *sb;
        return sb_impl.sbumpc();
    }

    template <class TStreambuf>
    static typename enable_if<is_base_of<
        streambuf_gptr_tag, TStreambuf>::value,
        typename TStreambuf::int_type>::type
    sungetc(TStreambuf* sb)
    {
        return sungetc_full(sb);
    }

    template <class TStreambuf>
    static typename enable_if<!is_base_of<
        streambuf_gptr_tag, TStreambuf>::value,
        typename TStreambuf::int_type>::type
    sungetc(TStreambuf* sb)
    {
        typename TStreambuf::impl_type& sb_impl = *sb;
        return sb_impl.sungetc();
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


}}}
