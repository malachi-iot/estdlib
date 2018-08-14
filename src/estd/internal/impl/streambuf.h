#pragma once

#include "../../traits/char_traits.h"
#include "../ios.h"
#include "../../queue.h" // for out_queue_streambuf

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


template <class T, size_t N>
struct layer1_queue_policy
{
    typedef layer1::deque<T, N> queue_type;
};


// EXPERIMENTAL
// but I don't expect API to change once it's settled, so not marking as experimental
template <class TWrappedStreambuf, class TPolicy = layer1_queue_policy<char, 64> >
struct out_queue_streambuf
{
    typedef typename TWrappedStreambuf::char_type char_type;
private:
    TWrappedStreambuf _rdbuf;

    typename TPolicy::queue_type put_area;

public:
    streamsize xsputn(const char_type* s, streamsize count)
    {
        while(count--)
        {
            put_area.push_back(*s++);
        }
    }

    int sync()
    {
        // TODO: Keep bumping into needing intrusive inspectors into queue to
        // get at underlying data buffers
        // _rdbuf.xsputn(put_area.data1());
    }
};


// EXPERIMENTAL
// Expect this to move to 'embr' namespace, though conceivably it could live here
// since it doesn't depend on any particular TNetbuf
// output streambuf whose output destination is a netbuf
// also trying to crowbar internal-pos tracking out of netbuf.  This is a good mechanism for it
// (i.e. 'processed' vs 'unprocessed' netbuf contents)
// note also netbuf architecture is such that it is decoupled from its consumer, so sync/emit
// calls to this streambuf would have diminished meaning unless we also connected this streambuf
// to an actual consuming device somehow.  Not a terrible idea, but not doing that for now
template <class TChar, class TNetbuf>
struct out_netbuf_streambuf
{
    typedef TChar char_type;
    typedef typename estd::remove_reference<TNetbuf> netbuf_type;
    typedef typename netbuf_type::size_type size_type;
private:

    // netbuf represents 'put area' in this context
    TNetbuf netbuf;

    // how far into current netbuf chunk we are
    size_type pos;

public:
    out_netbuf_streambuf(size_type pos = 0) : pos(pos) {}

    // NOTE: Duplicated code from elsewhere.  Annoying, but expected
    // since this is the first time I've put it in a truly standard place
    streamsize xsputn(const char_type* s, streamsize count)
    {
        char_type* d = netbuf.data() + pos;
        streamsize orig_count = count;
        size_type remaining = netbuf.length() - pos;

        // if we have more to write than fits in the current netbuf.data()
        while(count > remaining)
        {
            // put in as much as we can
            count -= remaining;
            while(remaining--) *d++ = *s++;

            // move to next netbuf.data()
            bool has_next = netbuf.next();
            // TODO: Assert that there's a next to work with

            // whether or not has_next succeeds, pos is reset here
            // this means if it fails, the next write operation will overwrite the contents
            // so keep an eye on your return streamsize
            pos = 0;

            if(has_next)
            {
                // if there's another netbuf.data() for us to move to, get specs for it
                remaining = netbuf.length();
                d = netbuf.data();
            }
            else
            {
                // NOTE: pos is left in an invalid state here
                // otherwise, we aren't able to write everything so return what we
                // could do
                return orig_count - count;
            }
        }

        pos += count;

        while(count--) *d++ = *s++;

        return orig_count;
    }
};


template <class TString>
struct basic_stringbuf
{
    typedef typename remove_reference<TString>::type string_type;
    typedef typename string_type::value_type char_type;
    typedef typename string_type::traits_type traits_type;

    TString _str;

    streamsize xsputn(const char_type* s, streamsize count)
    {
        // FIX: normal strings throw an exception if we exceed internal
        // buffer size, but here we should instead have an optional error
        // facility
        _str.append(s, count);
        return count;
    }

    // deviates from spec in that this is NOT a copy, but rather a direct reference
    // to the tracked string.  Take care
    string_type& str() { return _str; }
};

// this represents traditional std::basic_streambuf implementations
template <class TChar, class TCharTraits = ::std::char_traits<TChar> >
struct basic_streambuf
{
    typedef TChar char_type;
    typedef TCharTraits traits_type;
    typedef typename traits_type::int_type int_type;

protected:
    virtual streamsize xsgetn(char_type* s, streamsize count) = 0;
    virtual streamsize xsputn(const char_type* s, streamsize count) = 0;
};

}}}
