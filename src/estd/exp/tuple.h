#pragma once

#include "../variant.h"

namespace estd { namespace experimental {
// NOTE: this is experimental, pre C++03 support
#ifndef EXP1
template<class TArg1, class TArg2 = monostate, class TArg3 = monostate, class TArg4 = monostate>
class tuple;

namespace internal {

struct tuple_tag {};

template <class T, class TNext = tuple_tag>
class tuple_impl : public TNext
{
public:
    typedef T value_type;

    T value;

    typedef TNext next_type;

    TNext& next() { return *this; }

    inline tuple_impl() {}
    inline tuple_impl(const T& value) : value(value) {}

    template <class T2>
    inline tuple_impl(const T& v1, const T2& v2) :
        tuple_impl<T2>(v2),
        value(v1) {}

    template <class T2, class T3>
    inline tuple_impl(const T& v1, const T2& v2, const T3& v3) :
        tuple_impl<T2, tuple_impl<T3> >(v2, v3),
        value(v1) {}
};

template <size_t I, class T, class TNext>
struct _tuple_type_getter;

template <size_t I, class T, class TNext>
struct _tuple_type_getter
{
    typedef tuple_impl<T, TNext> impl_type;
};

template <class T, class TNext>
struct _tuple_type_getter<0, T, TNext>
{
    typedef tuple_impl<T, TNext> impl_type;
    typedef typename impl_type::value_type value_type;

    static value_type& value(impl_type& impl) { return impl.value; }
};

template <size_t I, class T, class TNext>
inline typename _tuple_type_getter<I, T, TNext>::value_type& get(tuple_impl<T, TNext>& impl)
{
    return _tuple_type_getter<I, T, TNext>::value(impl);
}

template <size_t I, class TImpl, bool Enable = estd::is_base_of<tuple_tag, TImpl>::value >
struct tuple_type_getter;

template <class T, class TNext>
struct tuple_type_getter<0, tuple_impl<T, TNext>, true>
{
    typedef tuple_impl<T, TNext> impl_type;
    typedef typename impl_type::value_type value_type;
    typedef value_type& reference;

    static reference value(impl_type& impl) { return impl.value; }
};

template <size_t I, class T, class TNext>
struct tuple_type_getter<I, tuple_impl<T, TNext>, true > :
    tuple_type_getter<I - 1, TNext>
{

};

template <size_t I, class TWrongImpl>
struct tuple_type_getter<I, TWrongImpl, false>
{
    typedef int& reference;
};

/*
template <size_t I>
void _iterate_test(monostate) {} */

template <size_t I, class T, class TNext>
    //typename = typename enable_if<I >= 0>::type>
    //typename enable_if<I >= 0, bool>::type = true>
//void
typename enable_if<(I > 0)>::type _iterate_test(tuple_impl<T, TNext>& t)
{
    //typedef typename tuple_impl<T, TNext>::next_type next_type;
    int val_ = I;
    _iterate_test<I - 1>(t.next());
}

template <size_t I, class T, class TNext>
    //typename = typename enable_if<I < 0>::type>
    //typename enable_if<I < 0, bool>::type = true>
//void
typename enable_if<I == 0>::type _iterate_test(tuple_impl<T, TNext>& t)
{
}

/*
template <std::size_t I, class TRet, class T, class TNext,
    typename enable_if<is_same<T, monostate>::value, bool>::type = true>
inline TRet&  _get_from_tuple(tuple_impl<T, TNext>& v)
{

}

template <std::size_t I, class TRet, class T, class TNext,
    typename enable_if<!is_same<T, monostate>::value, bool>::type = true>
inline TRet&  _get_from_tuple(tuple_impl<T, TNext>& v)
{

} */

}

template <class TArg1>
class tuple<TArg1,
        monostate,
        monostate,
        monostate>
{
public:
    typedef internal::tuple_impl<TArg1, monostate> impl_type;

    impl_type impl;

    tuple() {}
    tuple(const TArg1& v1) : impl(v1) {};
};

template <class TArg1, class TArg2>
class tuple<TArg1,
    TArg2,
    monostate,
    monostate>
{
public:
    typedef internal::tuple_impl<TArg1,
        internal::tuple_impl<TArg2> > impl_type;

    impl_type impl;

public:
    tuple() {}
    tuple(const TArg1& v1, const TArg2& v2) : impl(v1, v2) {};
};


template <class TArg1, class TArg2, class TArg3>
class tuple<TArg1,
    TArg2,
    TArg3,
    monostate>
{
public:
    typedef     internal::tuple_impl<TArg1,
        internal::tuple_impl<TArg2,
            internal::tuple_impl<TArg3> > > impl_type;

    impl_type impl;

public:
    tuple() {}
    tuple(const TArg1& v1, const TArg2& v2, const TArg3& v3) :
        impl(v1, v2, v3) {};
};

template <std::size_t I, class TTuple,
    class Getter = internal::tuple_type_getter<I, typename TTuple::impl_type> >
inline typename Getter::reference get(TTuple& tuple)
{
    // If you see 'value' is not a member, that may be a guard against an invalid TTuple type
    // (see tuple_type_getter's tuple_tag filter)
    return Getter::value(tuple.impl);
}

#elif defined(EXP2)
template<class TArg1>
class tuple;

template<class TArg1, class TArg2>
class tuple;

#endif
}}