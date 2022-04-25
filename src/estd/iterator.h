#pragma once

#include "internal/iterator_standalone.h"
#include "istream.h"
#include "ostream.h"



// TODO: Might need a specialization for our accessor-related things. we'll see
namespace estd {
namespace experimental {

template<class TStreambuf>
class istreambuf_iterator
{
public:
    typedef typename TStreambuf::char_type char_type;
    typedef typename TStreambuf::traits_type traits_type;

    typedef TStreambuf streambuf_type;

    typedef istreambuf_iterator iterator;
    typedef char_type value_type;

private:

    streambuf_type* const rdbuf;
    char_type ch;

public:
    istreambuf_iterator() :
        rdbuf(NULLPTR)
    {
    }

    istreambuf_iterator(streambuf_type& s) : rdbuf(&s)
    {
        ch = rdbuf->sgetc();
    }

    template <class TIstreamBase>
    istreambuf_iterator(estd::internal::basic_istream<TStreambuf, TIstreamBase>& is) :
        rdbuf(is.rdbuf())
    {
        ch = rdbuf->sgetc();
    }

    istreambuf_iterator(streambuf_type* s) :
        rdbuf(s)
    {
        ch = rdbuf->sgetc();
    }

    istreambuf_iterator(const istreambuf_iterator& copy_from) = default;

    // prefix version
    iterator& operator++()
    {
        ch = rdbuf->snextc();

        return *this;
    }

    // postfix version
    iterator operator++(int)
    {
        ch = rdbuf->sbumpc();

        return *this;
    }


    value_type operator*() const
    {
        return ch;
    }

    // EXPERIMENTAL
    // since streambufs are generally a forward only creature, and cross-streambuf comparison's
    // aren't really viable, this mainly exists to compare against a NULL (end) iterator
    bool operator!=(const iterator& compare_to) const
    {
        return rdbuf != compare_to.rdbuf;
    }
};

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
    ostreambuf_iterator(estd::internal::basic_ostream<TStreambuf, TBase>& stream) :
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
