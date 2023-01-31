#pragma once

#include "../type_traits.h"
#include "locale.h"
#include <stdint.h>
#include "iosfwd.h"

namespace estd {

#ifndef FEATURE_ESTD_AGGRESIVE_BITFIELD
#define FEATURE_ESTD_AGGRESIVE_BITFIELD 1
#endif


// DEBT: Move ostream feature flags elsewhere
#ifndef FEATURE_ESTD_OSTREAM_SETW
#define FEATURE_ESTD_OSTREAM_SETW 1
#define FEATURE_ESTD_OSTREAM_SETFILL 1
#define FEATURE_ESTD_OSTREAM_SETIOSALIGN 1
#endif



class ios_base
{
public:
    typedef uint8_t fmtflags;

    // NOTE: Spec appears to conflict with itself.
    // https://en.cppreference.com/w/cpp/io/ios_base/fmtflags suggests we have carte blanche
    // do make these any values we wish, but
    // https://en.cppreference.com/w/cpp/locale/num_get/get strongly implies that 'dec'
    // is expected to be zero
    static CONSTEXPR fmtflags dec = 0x01;
    static CONSTEXPR fmtflags hex = 0x02;
    static CONSTEXPR fmtflags oct = 0x03;
    static CONSTEXPR fmtflags basefield = dec | hex;

    static CONSTEXPR fmtflags left = 0x08;
    static CONSTEXPR fmtflags right = 0x10;
    static CONSTEXPR fmtflags adjustfield = left | right;

    static CONSTEXPR fmtflags boolalpha = 0x20;
    static CONSTEXPR fmtflags unitbuf = 0x40;

    // TODO: Not yet used, right now we are generally hardcoded to skipws = on
    static CONSTEXPR fmtflags skipws = 0x80;


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

    // Non standard, experimental.  Reflects that we are in a wait state
    // to see if there is any more data.  Maps to 'showmanyc' value of 0.
    static CONSTEXPR iostate nodatabit = 0x08;

    typedef uint8_t seekdir;

    static CONSTEXPR seekdir beg = 0x00;
    static CONSTEXPR seekdir end = 0x01;
    static CONSTEXPR seekdir cur = 0x02;

private:
    struct
    {
#if FEATURE_ESTD_AGGRESIVE_BITFIELD
        fmtflags fmtfl_ : 8;
        iostate iostate_ : 4;
#else
        fmtflags fmtfl_;
        iostate iostate_;
#endif

    }   state_;

protected:
#if FEATURE_ESTD_OSTREAM_SETW
    // DEBT: Move this into ostream proper
    struct
    {
        unsigned width : 4;
        unsigned alignment : 1;         // 1 = left, 0 = right
        char fillchar : 6;          // + 32

    }   ostream_;
#endif

    static CONSTEXPR openmode _openmode_null = 0; // proprietary, default of 'text'

    // remove state, not official call
    // UNTESTED
    void unsetstate(iostate state)
    {
        state_.iostate_ &= ~state;
    }

public:
    // DEBT: Use initializer lists for compilers that have it
    //ios_base() : fmtfl(dec), _iostate(goodbit) {}
    ios_base()
    {
        state_.fmtfl_ = dec;
        state_.iostate_ = goodbit;
#if FEATURE_ESTD_OSTREAM_SETW
        ostream_.width = 0;
        ostream_.fillchar = (char) (' ' - 0x20); // DEBT
        ostream_.alignment = 0;
#endif
    }

    fmtflags setf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ |= flags; return prior; }

    fmtflags setf(fmtflags flags, fmtflags mask)
    {
        fmtflags prior = state_.fmtfl_;
        state_.fmtfl_ &= ~mask;
        state_.fmtfl_ |= flags;
        return prior;
    }

    fmtflags unsetf(fmtflags flags)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ &= ~flags; return prior; }

    fmtflags flags() const
    { return state_.fmtfl_; }

    fmtflags flags(fmtflags fmtfl)
    { fmtflags prior = state_.fmtfl_; state_.fmtfl_ = fmtfl; return prior; }

    iostate rdstate() const
    { return state_.iostate_; }

    void clear(iostate state = goodbit)
    { state_.iostate_ = state; }

    void setstate(iostate state)
    {
        state_.iostate_ |= state;
    }

    bool good() const
    { return rdstate() == goodbit; }

    bool bad() const
    { return rdstate() & badbit; }

    bool fail() const
    { return rdstate() & failbit || rdstate() & badbit; }

    bool eof() const
    { return rdstate() & eofbit; }

protected:
    // internal call which we may make a layer0 version for optimization
    bool is_unitbuf_set() const { return state_.fmtfl_ & unitbuf; }

};

// NOTE: these are not heeded quite yet
inline ios_base& unitbuf(ios_base& s)
{
    s.setf(ios_base::unitbuf);
    return s;
}

inline ios_base& nounitbuf(ios_base& s)
{
    s.unsetf(ios_base::unitbuf);
    return s;
}

}