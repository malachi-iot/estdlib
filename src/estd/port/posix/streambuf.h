#ifndef UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
#define UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H

// specifically for non-FEATURE_IOS_STREAMBUF_FULL mode

#include <stdio.h> // For POSIX modes
#include <stdint.h>

#include "../../internal/impl/streambuf.h"

namespace estd {

//typedef ::_IO_FILE TEST_STREAM_T;

#ifdef __EXP1
template<class TChar, class Traits = char_traits <TChar>>
class basic_streambuf : public experimental::basic_streambuf_embedded<TChar, TEST_STREAM_T, Traits>
{
public:
    typedef TChar char_type;
    typedef TEST_STREAM_T stream_t;


protected:
    typedef experimental::basic_streambuf_embedded<TChar, TEST_STREAM_T, Traits> base_t;

    // http://putka.upm.si/langref/cplusplus.com/reference/iostream/streambuf/sgetn/index.html
    // acts like many sbumpc calls
    streamsize xsgetn(char_type *s, streamsize count)
    {
        return fread(s, sizeof(TChar), count, &this->stream);
    }


    streamsize xsputn(const char_type *s, streamsize count)
    {
        return fwrite(s, sizeof(TChar), count, &this->stream);
    }

public:
    basic_streambuf(TEST_STREAM_T &stream) : base_t(stream)
    {}

    int_type sputc(char_type ch)
    {
        return fputc(ch, &this->stream);
    }

    int_type sbumpc()
    {
        return fgetc(&this->stream);
    }

    // http://putka.upm.si/langref/cplusplus.com/reference/iostream/streambuf/sgetn/index.html
    // acts like many sbumpc calls
    streamsize sgetn(char_type* s, streamsize count)
    {
        return xsgetn(s, count);
    }

    streamsize sputn(const char_type* s, streamsize count)
    {
        return xsputn(s, count);
    }
};
#else

namespace internal {

typedef ::FILE& posix_stream_t;

namespace impl {

struct posix_streambuf :
        native_streambuf_base<char, posix_stream_t, ::std::char_traits<char> >
{
    typedef native_streambuf_base<char, posix_stream_t, ::std::char_traits<char> > base_type;
    typedef base_type::char_type char_type;
    //typedef char char_type;

    streamsize xsgetn(char_type* s, streamsize count);
    streamsize xsputn(const char_type* s, streamsize count);

    int sgetc();
    int sbumpc();
    int sputc(char);

    // TODO: Might want this one day, but it will mandate carring that #include <stdio.h> around
    // though at the moment we have to do that anyway due to ::FILE
    //int_type sputc(char_type ch) { return fputc(ch, &this->stream); }

    posix_streambuf(posix_stream_t stream) : base_type(stream) {}
};

} }

// FIX: TChar and Traits evaporate, for the time being
// we'll want to repair that
template <class TChar, class Traits = ::std::char_traits<TChar> >
class posix_streambuf : public ::estd::internal::streambuf<internal::impl::posix_streambuf>
{
    typedef internal::streambuf<internal::impl::posix_streambuf> base_type;

public:
    posix_streambuf(internal::posix_stream_t stdstream) : base_type(stdstream) {}
};

#endif

}
//} }

#endif //UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
