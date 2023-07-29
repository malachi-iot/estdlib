/**
 *
 *
 * References:
 *
 * 1. https://en.cppreference.com/w/cpp/iterator/istreambuf_iterator
 * 2. https://en.cppreference.com/w/cpp/iterator/istreambuf_iterator/equal
 */
#pragma once

#include "internal/platform.h"
#include "internal/iterator_standalone.h"
#include "internal/iosfwd.h"
#include "internal/ios_base.h"
#include "internal/istream_iterator.h"
#include "internal/istreambuf_iterator.h"
#include "internal/ostream_iterator.h"

// TODO: Might need a specialization for our accessor-related things. we'll see
namespace estd {

template <class TStreambuf>
bool operator==(
    const istreambuf_iterator<TStreambuf>& lhs,
    const istreambuf_iterator<TStreambuf>& rhs)
{
    return lhs.equal(rhs);
}


template <class TStreambuf>
bool operator!=(
    const istreambuf_iterator<TStreambuf>& lhs,
    const istreambuf_iterator<TStreambuf>& rhs)
{
    return !lhs.equal(rhs);
}


template <class TStreambuf>
bool operator==(
    const istreambuf_iterator<TStreambuf>& lhs,
    const typename istreambuf_iterator<TStreambuf>::proxy& rhs)
{
    return lhs.equal(rhs);
}


template <class TStreambuf>
bool operator==(
    const typename istreambuf_iterator<TStreambuf>::proxy& lhs,
    const istreambuf_iterator<TStreambuf>& rhs)
{
    return lhs.equal(rhs);
}

namespace experimental {

struct default_sentinel_t {};
#ifdef ESTD_CPP_INLINE_VARIABLES
inline constexpr default_sentinel_t default_sentinel{};
#endif


template <class TStreambuf>
class ostreambuf_iterator
{
public:
    typedef std::output_iterator_tag iterator_category;
    typedef void value_type;

    typedef TStreambuf streambuf_type;
    typedef typename streambuf_type::char_type char_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::int_type int_type;

    streambuf_type* rdbuf;
    int_type last_written;

    ostreambuf_iterator() :
        rdbuf(NULLPTR),
        last_written(traits_type::eof())
    {

    }

    ostreambuf_iterator(streambuf_type* rdbuf) :
        rdbuf(rdbuf)
    {

    }

    template <class TBase>
    ostreambuf_iterator(estd::detail::basic_ostream<TStreambuf, TBase>& stream) :
        rdbuf(stream.rdbuf())
    {

    }

    ostreambuf_iterator& operator=(char_type c)
    {
        if(rdbuf)
            last_written = rdbuf->sputc(c);

        return *this;
    }

    // prefix version
    ostreambuf_iterator& operator++()
    {
        return *this;
    }

    // postfix version
    ostreambuf_iterator operator++(int)
    {
        return *this;
    }


    bool failed() const
    {
        return last_written != traits_type::eof();
    }
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
