#include "../identify_platform.h"

#if defined(ESTD_POSIX) || defined(ESP_OPEN_RTOS)

#include "../../streambuf.h"
#include "streambuf.h"

namespace estd { namespace internal { namespace impl {

//template<>
streamsize native_streambuf<char, posix_stream_t, ::std::char_traits<char> >::
    xsputn(const char_type* s, streamsize count)
{
    return fwrite(s, sizeof(char), count, &this->stream);
}


//template<>
streamsize native_streambuf<char, posix_stream_t, ::std::char_traits<char> >::
    xsgetn(char_type* s, streamsize count)
{
    return fread(s, sizeof(char), count, &this->stream);
}

//template<>
int native_streambuf<char, posix_stream_t, ::std::char_traits<char> >::sputc(char ch)
{
    return fputc(ch, &this->stream);
}


//template<>
int native_streambuf<char, posix_stream_t, ::std::char_traits<char> >::sbumpc()
{
    return fgetc(&this->stream);
}

//template<>
int native_streambuf<char, posix_stream_t, ::std::char_traits<char> >::sgetc()
{
    int c = fgetc(&this->stream);
    ungetc(c, &this->stream);
    return c;
}

} } }

#endif
