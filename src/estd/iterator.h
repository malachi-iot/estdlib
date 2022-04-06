#pragma once

#include "internal/platform.h"
//#include "istream.h"
#include "type_traits.h"

#ifdef FEATURE_STD_ITERATOR
#include <iterator>

namespace std {

// nonstandard, reverse iterator tag
struct reverse_iterator_tag       : public input_iterator_tag {};

}

#else
namespace std {

// picked up from LLVM.  Not really relied on by our own libs at this time
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag       : public input_iterator_tag {};
// nonstandard, reverse iterator tag
struct reverse_iterator_tag       : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag, public reverse_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

}

#endif

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

namespace experimental {

template<class TStreambuf>
class istreambuf_iterator
{
public:
    typedef typename TStreambuf::char_type char_type;
    typedef typename TStreambuf::traits_type traits_type;

    typedef TStreambuf streambuf_type;
private:

    streambuf_type* const rdbuf;

public:
    istreambuf_iterator() :
        rdbuf(NULLPTR)
    {
    }

    /*
    template <class TIstreamBase>
    istreambuf_iterator(estd::internal::basic_istream<TStreambuf, TIstreamBase>& is) :
        rdbuf(is.rdbuf())
    {
    } */
};

// Similar to boost's version, but we don't use a functor (maybe we should?)
// https://www.boost.org/doc/libs/1_67_0/libs/iterator/doc/html/iterator/specialized/filter.html
template <class TPredicate, class TBaseIterator>
class filter_iterator
{
    TPredicate predicate;
    TBaseIterator baseIterator;

    typedef filter_iterator iterator;
    typedef typename iterator_traits<TBaseIterator>::value_type value_type;
    typedef typename iterator_traits<TBaseIterator>::reference reference;

public:
    filter_iterator(TPredicate predicate, TBaseIterator baseIterator) :
        predicate(predicate),
        baseIterator(baseIterator)
    {

    }

    // prefix version
    iterator& operator++()
    {
        ++baseIterator;

        return *this;
    }

    // NOTE: This is kind of a bummer we have to carry TPredicate around too
    // postfix version
    iterator operator++(int)
    {
        iterator temp(*this);
        operator++();
        return temp;
    }

    value_type operator*()
    {
        return predicate(*baseIterator);
    }
};

template <class TPredicate, class TIterator>
filter_iterator<TPredicate, TIterator> make_filter_iterator
    (TPredicate predicate, TIterator it)
{
    return filter_iterator<TPredicate, TIterator>(predicate, it);
}

}

}
