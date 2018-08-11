#ifndef UTIL_EMBEDDED_OSTREAM_H
#define UTIL_EMBEDDED_OSTREAM_H

#ifdef ESP_OPEN_RTOS
#else
// ESP_OPEN_RTOS has some non-sprintf ways to convert numeric to strings
#define USING_SPRINTF
#endif


extern "C" {

#if defined(USING_SPRINTF) || defined(__POSIX__)
#include <inttypes.h>
#endif

#include <string.h>
#include <stdlib.h>

};

#include "streambuf.h"
#include "ios.h"
#include "string_convert.h"

namespace FactUtilEmbedded { namespace std {

template<class TChar, class traits = char_traits<TChar>>
class basic_ostream :
#ifdef FEATURE_IOS_STREAMBUF_FULL
        virtual
#endif
        public basic_ios<TChar, traits>
{
    typedef basic_ios<TChar> base_t;

public:
    typedef basic_ostream<TChar> __ostream_type;

    // When the time comes, these will replace the old virtual ones
    __ostream_type& write(const TChar* s, streamsize n)
    {
        this->rdbuf()->sputn(s, n);
        return *this;
    }


    __ostream_type& put(TChar ch)
    {
        if(this->rdbuf()->sputc(ch) == char_traits<TChar>::eof())
            this->setstate(base_t::eofbit);

        return *this;
    }

    //friend basic_ostream& operator<<(basic_ostream& (*__pf)(basic_ostream&));

    __ostream_type& operator<<(__ostream_type& (*__pf)(__ostream_type&))
    {
        return __pf(*this);
    }

    /*
    basic_ostream& operator<<(basic_ostream& (*__pf)(basic_ostream&))
    {
        return __pf(*this);
    }*/

#ifndef FEATURE_IOS_STREAMBUF_FULL
    typedef typename base_t::stream_t stream_t;

    basic_ostream(stream_t& stream) : base_t(stream) {}
#endif
};

typedef basic_ostream<char> ostream;

/*
 need to SFINAE (or similar) out particular types like char* from coming in here
*/
/*
template <class T>
inline ostream& operator<<(ostream& out, T value)
{
    char buffer[::experimental::maxStringLength<T>()];

    toString(buffer, value, sizeof(buffer) - 1);

    return out << buffer;
}
*/


inline ostream& operator <<(ostream& out, const char* arg)
{
    return out.write(arg, strlen(arg));
}

inline basic_ostream<char>& operator <<(basic_ostream<char>& out, char ch)
{
    return out.put(ch);
}


inline basic_ostream<char>& operator <<(basic_ostream<char>& out, uint16_t value)
{
    char buffer[10];

    toString(buffer, value, sizeof(buffer) - 1);

    return out << buffer;
}


inline basic_ostream<char>& operator <<(basic_ostream<char>& out, uint32_t value)
{
    char buffer[16];

#ifdef ESP_OPEN_RTOS
    __utoa(value, buffer, 10);
#else
    snprintf(buffer, sizeof(buffer), "%" PRIu32, value);
#endif

    return out << buffer;
}


inline basic_ostream<char>& operator<<(basic_ostream<char>& out, void* addr)
{
    char buffer[sizeof(uintptr_t) * 3];

#ifdef ESP_OPEN_RTOS
    __utoa((uint32_t)addr, buffer, 16);
#else
    snprintf(buffer, sizeof(buffer), "%" PRIXPTR, (uintptr_t)addr);
#endif
    return out << buffer;
}

inline basic_ostream<char>& operator<<(basic_ostream<char>& out, int value)
{
    char buffer[10];

    toString(buffer, value, sizeof(buffer) - 1);

    return out << buffer;
}

inline ostream& operator<<(ostream& out, float value)
{
    char buffer[::experimental::maxStringLength<float>()];

    toString(buffer, value, sizeof(buffer) - 1);

    return out << buffer;
}


inline basic_ostream<char>& endl(basic_ostream<char>& __os)
{ return __os.put('\n'); }


inline basic_ostream<char>& dec(basic_ostream<char>& __os)
{
    __os.flags((__os.flags() & ~ios_base::basefield) | ios_base::dec);
    return __os;
}

inline basic_ostream<char>& hex(basic_ostream<char>& __os)
{
    __os.flags((__os.flags() & ~ios_base::basefield) | ios_base::hex);
    return __os;
}


} }

#ifdef ARDUINO
#include "streams/ostream_arduino.h"
#endif

#endif //UTIL_EMBEDDED_OSTREAM_H