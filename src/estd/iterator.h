#pragma once

#include "internal/platform.h"
#include "type_traits.h"

#ifdef FEATURE_STD_ITERATOR
#include <iterator>
#else
namespace std {

// picked up from LLVM.  Not really relied on by our own libs at this time
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag       : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag, public reverse_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

}

#endif

namespace std {

// nonstandard, reverse iterator tag
struct reverse_iterator_tag       : public input_iterator_tag {};

}

namespace estd {

template< class InputIt >
//typename std::iterator_traits<InputIt>::difference_type
int
distance( InputIt first, InputIt last )
{
    int count = 0;
    while(first != last)
    {
        count++;
        first++;
    }
    //std::iterator_traits<InputIt>::difference_type count = 0;

    return count;
}

template <class T>
struct iterator_traits
{
    typedef typename T::difference_type difference_type;
    typedef typename T::value_type value_type;
    typedef typename T::pointer pointer;
    typedef typename T::reference reference;
    typedef typename T::iterator_category iterator_category;
};

template <class T>
struct iterator_traits<T*>
{
    typedef std::ptrdiff_t difference_type;
    // jump right to c++20 way of doing things
    typedef typename estd::remove_cv<T>::type value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::random_access_iterator_tag iterator_category;
};

// TODO: Might need a specialization for our accessor-related things. we'll see

}
