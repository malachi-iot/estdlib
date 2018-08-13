#pragma once

#include "../../traits/char_traits.h"
#include "../ios.h"

namespace estd { namespace internal { namespace impl {

// NOTE: Temporarily making a 'complete' type, but plan is to make this always specialized
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf;

// Expect most native stream implementations can use this helper base impl
template <class TChar, class TStream, class TCharTraits >
struct native_streambuf_base
{
    // NOTE: we'll need to revisit this if we want a proper pointer in here
    typedef typename estd::remove_reference<TStream>::type stream_type;
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    TStream stream;

    native_streambuf_base(stream_type& stream) : stream(stream)
            {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    native_streambuf_base(stream_type&& move_from) : stream(std::move(move_from)) {}
#endif
};

// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits = ::std::char_traits<TChar> >
struct basic_streambuf
{
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    virtual streamsize xsgetn(TChar* s, streamsize count) = 0;
    virtual streamsize xsputn(const TChar* s, streamsize count) = 0;
};

}}}
