#pragma once

#include "ios.h"

namespace estd { namespace detail {

// DEBT: it's time to put this into 'detail' namespace, since it's used much more often
// than estd::basic_ostream - is there a way to make TBase a little less confusing for consumers?
template <class TStreambuf, class TBase>
class basic_ostream :
#ifdef FEATURE_IOS_STREAMBUF_FULL
        virtual
#endif
        public TBase
{
    typedef TBase base_type;

public:
    typedef typename base_type::streambuf_type streambuf_type;
    typedef typename TBase::char_type char_type;
    typedef typename streambuf_type::pos_type pos_type;
    typedef typename streambuf_type::off_type off_type;

private:
    //typedef experimental::ios_policy policy_type;
    //typedef int policy_type;
    typedef typename base_type::policy_type policy_type;

#ifdef FEATURE_ESTD_OSTREAM_TIMEOUT
    policy_type get_policy() { return policy_type{}; }

    template <class TPolicy = policy_type,
              class Enabled = typename TPolicy::do_timeout_tag>
    void write_timeout(const char_type* s, streamsize n)
    {
        using namespace chrono;

        milliseconds timeout(get_policy().timeout_in_ms());
        milliseconds sleep_for(get_policy().sleep_in_ms());
        typedef steady_clock::time_point time_point;
        time_point start = steady_clock::now();
        milliseconds elapsed;

        streamsize remaining = n;

        do
        {
            streamsize written = this->rdbuf()->sputn(s, n);

            remaining -= written;
            s += written;

            if(remaining > 0 && sleep_for.count() > 0)
            {
                this_thread::sleep_for(sleep_for);
            }
            else
            {
                this_thread::yield();
            }

            // FIX: Can't quite do this because std::duration doesn't
            // interact with estd::duration well and implicit conversions
            // aren't kicking in presumably due to the templates
            /*
            if(steady_clock::now() - start > timeout)
            {

            } */

            elapsed = duration_cast<milliseconds>(steady_clock::now() - start);
        }
        while(remaining > 0 && elapsed < timeout);
    }

    template <class TPolicy = policy_type,
              class Enabled = typename enable_if<!experimental::is_do_timeout_tag_present<TPolicy>::value>::type>
#endif
    void write_timeout(const char_type* s, streamsize n, bool = false)
    {
        streamsize written = this->rdbuf()->sputn(s, n);

        if (written != n)
            base_type::setstate(ios_base::failbit);
    }

public:
    struct sentry
    {
        explicit sentry(basic_ostream&) {}

        // NOTE: deviate from spec so that we don't risk extra stuff floating
        // around on stack.  This won't always be convenient though
        // NOTE also we might opt for a compile-time flag on unitbuf to further
        // optimize things
        inline static void destroy(basic_ostream& os)
        {
            //if(os.flags() & ios_base::unitbuf)
            if (os.is_unitbuf_set() && os.good())
                os.flush();
        }

        ~sentry()
        {

        }
    };

    typedef typename TBase::traits_type traits_type;

    typedef basic_ostream<TStreambuf, TBase> __ostream_type;

    __ostream_type& flush()
    {
        if (this->rdbuf()->pubsync() == -1)
            this->setstate(base_type::badbit);

        return *this;
    }

    // UNTESTED
    __ostream_type& seekp(off_type off, ios_base::seekdir dir)
    {
        this->rdbuf()->pubseekoff(off, dir, ios_base::out);
    }

    // When the time comes, these will replace the old virtual ones
    __ostream_type& write(const char_type* s, streamsize n)
    {
        write_timeout(s, n);
        sentry::destroy(*this);
        return *this;
    }


    __ostream_type& put(char_type ch, bool bypass_sentry = false)
    {
        if (this->rdbuf()->sputc(ch) == estd::char_traits<char_type>::eof())
            this->setstate(base_type::eofbit);

        if (!bypass_sentry)
            sentry::destroy(*this);

        return *this;
    }

    // NOTE: Our tellp will indeed return an offset output position indicator
    // on streambufs that can handle it, such as string, span and netbuf - they are not
    // fully implemented for that yet however (just string)
    pos_type tellp()
    {
        if (this->fail() == true) return pos_type(-1);

        return this->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
    }

    //friend basic_ostream& operator<<(basic_ostream& (*__pf)(basic_ostream&));

    __ostream_type& operator<<(__ostream_type& (* __pf)(__ostream_type&))
    {
        return __pf(*this);
    }

    /*
    basic_ostream& operator<<(basic_ostream& (*__pf)(basic_ostream&))
    {
        return __pf(*this);
    }*/

#ifndef FEATURE_IOS_STREAMBUF_FULL
    ESTD_CPP_FORWARDING_CTOR(basic_ostream)
#endif

};

} // detail

namespace internal {

// Internal call - write an integer of the specified base to the output stream
// DEBT: No locale num_put available yet.
// to_string_opt is less overhead so really we'd like to compile time choose
// one or the other
template <unsigned base, class TStreambuf, class TBase, class T>
inline basic_ostream<TStreambuf, TBase>& write_int(basic_ostream<TStreambuf, TBase>& out, T value)
{
    // +1 for potential - sign
    // +1 for null terminator
    char buffer[estd::numeric_limits<T>::template length<base>::value + 2];

    to_chars_result result = to_string_opt(buffer, value, base);

    int sz = &buffer[sizeof(buffer) - 1] - result.ptr;

    return out.write(result.ptr, sz);
}

template <class TStreambuf, class TBase, typename TInt>
basic_ostream<TStreambuf, TBase>& out_int_helper(basic_ostream<TStreambuf, TBase>& out, TInt value)
{
    // DEBT: another typical enum -> traits/template conversion - a framework
    // support for that really would be useful
    switch(out.flags() & ios_base::basefield)
    {
        case ios_base::oct:
            return write_int<8>(out, value);

        case ios_base::dec:
            return write_int<10>(out, value);

        case ios_base::hex:
            return write_int<16>(out, value);

        default:
            // TODO: assert or log an error condition
            return out;
    }
}


}}
