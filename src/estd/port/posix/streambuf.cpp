#include "../identify_platform.h"

#if defined(ESTD_POSIX) || defined(ESP_OPEN_RTOS)

#include <stdio.h>
#include "../../streambuf.h"

namespace estd { namespace internal {

template<>
streamsize streambuf<char, TEST_STREAM_T>::
    xsputn(const char* s, streamsize count)
{
    return fwrite(s, sizeof(char), count, &this->stream);
}


template<>
streamsize streambuf<char, TEST_STREAM_T>::
    xsgetn(char* s, streamsize count)
{
    return fread(s, sizeof(char), count, &this->stream);
}

template<>
int streambuf<char, TEST_STREAM_T>::sputc(char ch)
{
    return fputc(ch, &this->stream);
}


template<>
int streambuf<char, TEST_STREAM_T>::sbumpc()
{
    return fgetc(&this->stream);
}

template<>
int streambuf<char, TEST_STREAM_T>::sgetc()
{
    int c = fgetc(&this->stream);
    ungetc(c, &this->stream);
    return c;
}

} }

#endif
