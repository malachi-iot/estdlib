#pragma once

// NOTE: this is experimental, pre C++03 support
#ifdef EXP1
template<class TArg1, class TArg2, class TArg3, class TArg4>
class tuple;

namespace internal {

struct empty_value {};

}

template <class TArg1>
class tuple<TArg1,
        internal::empty_value,
        internal::empty_value,
        internal::empty_value>
{

};
#elif defined(EXP2)
template<class TArg1>
class tuple;

template<class TArg1, class TArg2>
class tuple;

#endif
