// Pulled in from util.embedded, but not yet adapted to estd environment

#pragma once

#include "streambuf.h"

namespace estd {

class ios_base
{
public:
    typedef uint8_t fmtflags;

    static CONSTEXPR fmtflags dec = 0x01;
    static CONSTEXPR fmtflags hex = 0x02;
    static CONSTEXPR fmtflags unitbuf = 0x04;
    static CONSTEXPR fmtflags basefield = dec | hex;

    typedef uint8_t openmode;

    static CONSTEXPR openmode app = 0x01;
    static CONSTEXPR openmode binary = 0x02;
    static CONSTEXPR openmode in = 0x04;
    static CONSTEXPR openmode out = 0x08;

    typedef uint8_t iostate;

    static CONSTEXPR iostate goodbit = 0x00;
    static CONSTEXPR iostate badbit = 0x01;
    static CONSTEXPR iostate failbit = 0x02;
    static CONSTEXPR iostate eofbit = 0x04;

private:
    fmtflags fmtfl;
    iostate _iostate;

protected:
    static constexpr openmode _openmode_null = 0; // proprietary, default of 'text'

public:
    fmtflags flags() const
    { return fmtfl; }

    fmtflags flags(fmtflags fmtfl)
    { return this->fmtfl = fmtfl; }

    iostate rdstate() const
    { return _iostate; }

    void clear(iostate state = goodbit)
    { _iostate = state; }

    void setstate(iostate state)
    {
        _iostate |= state;
    }

    bool good() const
    { return rdstate() == goodbit; }

    bool bad() const
    { return rdstate() & badbit; }

    bool fail() const
    { return rdstate() & failbit || rdstate() & badbit; }

    bool eof() const
    { return rdstate() & eofbit; }
};

}

#include "internal/ios.h"

namespace estd {

// TODO: hardwire in more of a explicit 'posix_streambuf' or whatever based on platform, since proper
// std::ios stuff basic_streambuf is a polymorphic base class.  That or perhaps do away with ios
// typedef altogether
template<class TChar, class Traits = ::std::char_traits<TChar> >
using basic_ios = estd::internal::basic_ios<basic_streambuf <TChar, Traits> >;

typedef
basic_ios<char> ios;

}
