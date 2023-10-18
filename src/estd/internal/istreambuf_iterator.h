#pragma once


#include "platform.h"
#include "iterator_standalone.h"
#include "iosfwd.h"
#include "ios_base.h"


// TODO: Might need a specialization for our accessor-related things. we'll see
namespace estd {

template<class TStreambuf>
class istreambuf_iterator
{
public:
    typedef typename TStreambuf::char_type char_type;
    typedef typename TStreambuf::traits_type traits_type;

    typedef TStreambuf streambuf_type;

    typedef istreambuf_iterator iterator;
    typedef typename traits_type::int_type int_type;
    /// Deviates from spec in that we return int_type and not char_type so as to reflect
    /// underlying stream eof condition (remember that only means no more data AT THE MOMENT)
    typedef int_type value_type;

    class proxy;

private:
    friend class proxy;

    // Can be null if representing an 'end' iterator
    streambuf_type* rdbuf_;

    //bool end() const { return ch == traits_type::eof(); }
    //bool end() const { return rdbuf_->in_avail() == -1; }
    ESTD_CPP_CONSTEXPR_RET bool end() const { return rdbuf_ == NULLPTR; }


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
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    istreambuf_iterator() :
        rdbuf_(NULLPTR)
    {
    }

    istreambuf_iterator(streambuf_type& s) : rdbuf_(&s)
    {
    }

    template <class TIstreamBase>
    istreambuf_iterator(estd::detail::basic_istream<TStreambuf, TIstreamBase>& is) :
        rdbuf_(is.rdbuf())
    {
    }

    istreambuf_iterator(streambuf_type* s) :
        rdbuf_(s)
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
            int_type _ch = rdbuf_->snextc();

            if (_ch == traits_type::eof() && rdbuf_->in_avail() == -1)
                rdbuf_ = NULLPTR;
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
            proxy p(rdbuf_->sgetc(), *this);

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
        return rdbuf_->sgetc();
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

}