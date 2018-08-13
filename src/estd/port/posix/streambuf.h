#ifndef UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
#define UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H

// specifically for non-FEATURE_IOS_STREAMBUF_FULL mode

#include <stdio.h> // For POSIX modes
#include <stdint.h>

namespace estd {
//namespace FactUtilEmbedded { namespace std {

typedef ::FILE& TEST_STREAM_T;
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

template <class TChar, class Traits = ::std::char_traits<TChar>>
using basic_streambuf = ::estd::internal::streambuf<TChar, TEST_STREAM_T, Traits>;

template <class TChar, class Traits = ::std::char_traits<TChar> >
class posix_streambuf : public ::estd::internal::native_streambuf<TChar, TEST_STREAM_T, Traits>
{

};

#endif

}
//} }

#endif //UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
