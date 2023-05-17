#pragma once

#include "../type_traits.h"
#include "../internal/fwd/c++03-tuple.h"

namespace estd {

namespace internal {

struct tuple_tag {};

template <size_t I, class TImpl>
struct tuple_navigator;


template <class T, class TNext = tuple_tag>
class tuple_impl : public TNext
{
public:
    typedef T value_type;
    typedef TNext next_type;

protected:
    // Experimenting with keeping this inside impl class.  Works identically
    // to tuple_type_getter
    template <std::size_t I, typename Enabled = void>
    struct navigator2;

    template <std::size_t I>
    struct navigator2<I, typename estd::enable_if<(I == 0)>::type>
    {
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef tuple_impl impl_type;

        static const_reference value(const impl_type& impl) { return impl.value; }
        static reference value(impl_type& impl) { return impl.value; }
        static void value(impl_type& impl, const_reference v) { impl.value = v; }
    };

    template <std::size_t I>
    struct navigator2<I, typename estd::enable_if<(I > 0)>::type> :
        next_type::template navigator2<I - 1>
    {

    };

public:
    T value;

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

    template <size_t I>
    struct navigator : tuple_navigator<I, tuple_impl> {};

    template <std::size_t I>
    inline typename navigator<I>::reference get()
    {
        // If you see 'value' is not a member, that may be a guard against an invalid TTuple type
        // (see tuple_type_getter's tuple_tag filter)
        return navigator<I>::value(*this);
    }

    template <std::size_t I>
    inline typename navigator<I>::const_reference get() const
    {
        // If you see 'value' is not a member, that may be a guard against an invalid TTuple type
        // (see tuple_type_getter's tuple_tag filter)
        return navigator<I>::value(*this);
    }
};

template <class T, class TNext>
struct tuple_navigator<0, tuple_impl<T, TNext> >
{
    typedef tuple_impl<T, TNext> impl_type;
    typedef typename impl_type::value_type value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    static const_reference value(const impl_type& impl) { return impl.value; }
    static reference value(impl_type& impl) { return impl.value; }
    static void value(impl_type& impl, const_reference v) { impl.value = v; }
};

template <size_t I, class T, class TNext>
struct tuple_navigator<I, tuple_impl<T, TNext> > :
    tuple_navigator<I - 1, TNext>
{

};


template <size_t I, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
struct tuple_navigator<I, tuple<Arg1, Arg2, Arg3, Arg4, Arg5> > :
    tuple_navigator<I, typename tuple<Arg1, Arg2, Arg3, Arg4, Arg5>::impl_type >
{

};

template <size_t I, class TWrongImpl>
struct tuple_navigator
{
    typedef int& reference;

    // This is as close as we can get to a custom error indicating 'tuple' wasn't correctly
    // fed into the getter.  'invalid_tuple' is expected to not exist and generate an error.
    static reference value(TWrongImpl) { return TWrongImpl::invalid_tuple(); }
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

}

template <class TArg1>
class tuple<TArg1>
{
public:
    typedef internal::tuple_impl<TArg1> impl_type;

    impl_type impl;

    tuple() {}
    tuple(const TArg1& v1) : impl(v1) {};
};

template <class TArg1, class TArg2>
class tuple<TArg1, TArg2>
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
class tuple<TArg1, TArg2, TArg3>
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


template <class TArg1, class TArg2, class TArg3, class TArg4>
class tuple<TArg1, TArg2, TArg3, TArg4>
{
public:
    typedef     internal::tuple_impl<TArg1,
        internal::tuple_impl<TArg2,
            internal::tuple_impl<TArg3,
                internal::tuple_impl<TArg4> > > > impl_type;

    impl_type impl;

public:
    tuple() {}
    tuple(const TArg1& v1, const TArg2& v2, const TArg3& v3, const TArg4& v4) :
        impl(v1, v2, v3, v4) {};
};

// DEBT: Need to narrow down 'get' here so that non-tuple flavors of get don't collide

template <std::size_t I, class TTuple>
inline typename internal::tuple_navigator<I, TTuple>::reference get(TTuple& tuple)
{
    return tuple.impl.template get<I>();
}


template <std::size_t I, class TTuple>
inline typename internal::tuple_navigator<I, TTuple>::const_reference get(const TTuple& tuple)
{
    return tuple.impl.template get<I>();
}


template <std::size_t I, class TTuple>
inline void set(TTuple& tuple, typename internal::tuple_navigator<I, TTuple>::const_reference v)
{
    // If you see 'value' is not a member, that may be a guard against an invalid TTuple type
    // (see tuple_type_getter's tuple_tag filter)
    internal::tuple_navigator<I, TTuple>::value(tuple.impl, v);
}

}