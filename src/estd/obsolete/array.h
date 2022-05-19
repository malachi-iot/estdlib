#pragma once

namespace estd {
#ifdef FEATURE_CPP_VARIADIC
namespace experimental { namespace layer0 {

// fiddling with idea suggested here:
// https://stackoverflow.com/questions/19019252/create-n-element-constexpr-array-in-c11

template <class T, T... values>
class array_exp1
{
public:
    typedef T value_type;
};

template <class T, T value, T... values>
class array_exp1<T, value, values...> : array_exp1<T, values...>
{
    typedef array_exp1<T, values...> base_t;

public:
    typedef T value_type;
};


template <class T, T... values>
class array
{
public:
    typedef T value_type;
};

template <class T, T value, T... values>
class array<T, value, values...> : array<T, values...>
{

};

// since this generates annoying warnings under some circumstances (esp32),
// AND it's experimental, only enabling during unit testing
#ifdef UNIT_TESTING
// FIX: So far is not viable under gcc, but not sure why.  Merely rejects
// anything coming into 'value'.  Maybe because 'T* const'?
template <class T, T* const value, size_t N>
struct array_exp2 :
        array_base<T, T* const,
            size_t,
            //typename estd::internal::deduce_fixed_size_t<N>::type,
            global_pointer_provider<T, value> >
{
    typedef array_base<T, T* const,
        size_t,
        //typename estd::internal::deduce_fixed_size_t<N>::type,
        global_pointer_provider<T, value> > base_type;
    typedef typename base_type::size_type size_type;
    //typedef typename base_type::const_iterator const_iterator;

    typedef const T* const const_iterator;
    typedef T* iterator;

    // just because pointer casting is different, it creates some warnings, so brute
    // force code again to reduce those warnings
    // esp32 generates its own set of compiler warnings from this though
    iterator begin() { return base_type::data(); }
    const_iterator begin() const { return base_type::data(); }

    iterator end() { return base_type::data() + N; }
    const_iterator end() const { return base_type::data() + N; }
};
#endif

/*
 * Won't work because 'array' would need to be a constexpr, which is
 * invalid as a function parameter modifier
template <class T, size_t N>
constexpr auto make_array(T (&array)[N]) -> array_exp2<T, array, N>
{
    return array_exp2<T, array, N>(array);
} */

/*
template <class T, class TArray, size_t N = sizeof(TArray) / sizeof(T)>
constexpr array_exp2<T, TArray, N> make_array()
{
}
*/
}}
#endif
}