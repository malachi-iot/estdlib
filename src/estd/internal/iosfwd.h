#pragma once

namespace estd {
    
namespace internal {

template <class TStreambuf, class TBase>
class basic_ostream;

template <class TStreambuf, class TBase>
class basic_istream;

}

template <class TStreambuf, class TBase>
internal::basic_istream<TStreambuf, TBase>& 
    ws(internal::basic_istream<TStreambuf, TBase>& __is);

}