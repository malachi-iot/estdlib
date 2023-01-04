#pragma once

// this is for forwards of non std ostream stuff that wouldn't be found in normal iosfwd.h

#include "../charconv.h"
#include "../iosfwd.h"

namespace estd {

namespace internal {

template <unsigned N, typename TInt>
inline to_chars_result to_string_opt(char (&buffer)[N], TInt value, unsigned base);

template <unsigned base, unsigned N, class TStreambuf, class TBase, class T>
inline basic_ostream<TStreambuf, TBase>& write_int(basic_ostream<TStreambuf, TBase>& out, T value);

template <class TStreambuf, class TBase, typename TInt>
basic_ostream<TStreambuf, TBase>& out_int_helper(basic_ostream<TStreambuf, TBase>& out, TInt value);

}

}