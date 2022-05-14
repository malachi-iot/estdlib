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



// TODO: Might need a specialization for our accessor-related things. we'll see
namespace estd {
namespace experimental {

struct default_sentinel_t {};
#ifdef ESTD_CPP_INLINE_VARIABLES
inline constexpr default_sentinel_t default_sentinel{};
#endif

template<class TStreambuf>
class istreambuf_iterator
{
public:
    typedef typename TStreambuf::char_type char_type;
    typedef typename TStreambuf::traits_type traits_type;

    typedef TStreambuf streambuf_type;

    typedef istreambuf_iterator iterator;
    typedef char_type value_type;
    typedef typename traits_type::int_type int_type;

    class proxy;

private:
    friend class proxy;

    streambuf_type* rdbuf;

    //bool end() const { return ch == traits_type::eof(); }
    bool end() const { return rdbuf == NULLPTR; }


    // https://en.cppreference.com/w/cpp/iterator/istreambuf_iterator/equal
    // [2] Tells suggests that only validity of the streambufs are of interest here
    // [1] Tells us that EOF is of interest also
    template <class TLHS, class TRHS>
    static bool equal(const TLHS& lhs, const TRHS& rhs)
    {
        if(lhs.end() && rhs.end()) return true;

        if(!lhs.end() && !rhs.end()) return true;

        return false;
    }

public:
    istreambuf_iterator() :
        rdbuf(NULLPTR)
    {
    }

    istreambuf_iterator(streambuf_type& s) : rdbuf(&s)
    {
    }

    template <class TIstreamBase>
    istreambuf_iterator(estd::internal::basic_istream<TStreambuf, TIstreamBase>& is) :
        rdbuf(is.rdbuf())
    {
    }

    istreambuf_iterator(streambuf_type* s) :
        rdbuf(s)
    {
    }

#ifdef FEATURE_CPP_DEFAULT_CTOR
    istreambuf_iterator(const istreambuf_iterator& copy_from) = default;
#endif

    // prefix version
    iterator& operator++()
    {
        if(!end())
        {
            // Prefix operator, we want to semi-peek into the next character to get a line
            // in on whether we're EOF
            int_type _ch = rdbuf->snextc();

            if (_ch == traits_type::eof())
                rdbuf = NULLPTR;
        }

        return *this;
    }

    class proxy
    {
        friend class istreambuf_iterator;

        const int_type ch;
        iterator& source;

        bool end() const { return ch == traits_type::eof(); }

        proxy(int_type ch, iterator& source) : ch(ch), source(source) {}

    public:
        char_type operator*() const
        {
            return traits_type::to_char_type(ch);
        }

        proxy operator++(int) { return source.operator++(int()); }
        iterator operator++() { return source.operator++(); }

        bool equal(const iterator& it) const
        {
            return iterator::equal(*this, it);
        }
    };

    // postfix version
    proxy operator++(int)
    {
        if(!end())
        {
            proxy p(rdbuf->sgetc(), *this);

            operator++();

            return p;
        }
        else
        {
            return proxy(traits_type::eof(), *this);
        }

    }


    value_type operator*() const
    {
        return rdbuf->sgetc();
    }

    bool equal(const iterator& it) const
    {
        return equal(*this, it);
    }


    bool equal(const proxy& it) const
    {
        return equal(*this, it);
    }
};

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
