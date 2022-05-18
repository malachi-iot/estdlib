#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
//#include "../ios.h"
#include "../type_traits.h"
#include "locale.h"
#include <stdint.h>
#include "../iosfwd.h"
#include "../variant.h"
#include "../thread.h"
#include "ios_base.h"
#include "ios_policy.h"

#include "locale/ctype.h"
#include "locale/facet.h"


namespace estd {

namespace experimental {

// 21MAR21 TODO: Revisit this and combine with new ios_base_policy
struct ios_policy
{
    // whether to do timeouts at all
    // for now timeout parameters apply equally to both istream and ostream
    typedef void do_timeout_tag;

    // whether to do yield/sleeps while polling
    typedef void do_sleep_tag;

    // doing a timeout also means that we attempt retries on read/write operations
    static CONSTEXPR bool do_timeout() { return true; }

    static CONSTEXPR size_t timeout_in_ms() { return 1000; }

    static CONSTEXPR size_t sleep_in_ms() { return 1000; }
};

template <typename>
struct has_typedef { typedef void type; };

template<typename T, typename = void>
struct is_do_timeout_tag_present : estd::false_type {};

template<typename T>
struct is_do_timeout_tag_present<T, typename has_typedef<typename T::do_timeout_tag>::type> : estd::true_type {};


}

namespace internal {

// eventually, depending on layering, we will use a pointer to a streambuf or an actual
// value of streambuf itself
template <class TStreambuf, bool use_pointer>
class basic_ios_base;

template <class TStreambuf>
class basic_ios_base<TStreambuf, true> : public ios_base
{
protected:
    typedef typename remove_reference<TStreambuf>::type streambuf_type;
    streambuf_type* _rdbuf;

    streambuf_type* rdbuf() const { return _rdbuf; }

protected:
    basic_ios_base(streambuf_type* sb) : _rdbuf(sb) {}

    void init(streambuf_type* sb)
    {
        _rdbuf = sb;
    }

    streambuf_type* rdbuf(streambuf_type* sb)
    {
        clear();
        streambuf_type temp = _rdbuf;
        _rdbuf = sb;
        return temp;
    }
};


// this one assumes for now our special 'native_streambuf' which shall be the
// de-specialized version of our basic_streambuf
template <class TStreambuf>
class basic_ios_base<TStreambuf, false> : public ios_base
{
public:
    typedef typename remove_reference<TStreambuf>::type streambuf_type;

protected:
    TStreambuf _rdbuf;

    basic_ios_base() {}

    // TODO: constructor needs cleanup here

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_ios_base(TArgs&&...args) : 
        _rdbuf(std::forward<TArgs>(args)...) {}

    basic_ios_base(streambuf_type&& streambuf) :
        _rdbuf(std::move(streambuf))    {}
#else
    template <class TParam1>
    basic_ios_base(TParam1& p1) : _rdbuf(p1)
            {}
#endif
    basic_ios_base(streambuf_type& streambuf) :
        _rdbuf(streambuf) {}

public:
    streambuf_type* rdbuf()
    { return &_rdbuf; }
};


//template<class TChar, class Traits = std::char_traits <TChar>>
template<class TStreambuf, bool use_pointer = false,
    class TPolicy = ios_base_policy<TStreambuf> >
class basic_ios : public basic_ios_base<TStreambuf, use_pointer>,
    estd::internal::struct_evaporator<TPolicy>,
    estd::internal::struct_evaporator<typename TPolicy::locale_type>
{
public:
    typedef basic_ios_base<TStreambuf, use_pointer> base_type;
    typedef typename base_type::streambuf_type streambuf_type;
    typedef typename streambuf_type::traits_type traits_type;
    typedef typename traits_type::char_type char_type;

    typedef TPolicy policy_type;
    typedef typename policy_type::locale_type locale_type;
    typedef typename policy_type::blocking_type blocking_type;

    typedef typename estd::internal::struct_evaporator<TPolicy> policy_provider_type;
    typedef typename policy_provider_type::evaporated_type evaporated_policy_type;
    typedef typename estd::internal::struct_evaporator<locale_type> locale_provider_type;

protected:
    basic_ios() {}

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    basic_ios(TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...) {}

    basic_ios(streambuf_type&& streambuf) :
        base_type(std::move(streambuf)) {}
#else
    template <class TParam1>
    basic_ios(TParam1& p) : base_type(p) {}

    template <class TParam1>
    basic_ios(TParam1* p) : base_type(p) {}
#endif
    basic_ios(streambuf_type& streambuf) :
        base_type(streambuf) {}

public:
    evaporated_policy_type policy() const { return policy_provider_type::value(); }
    
    // NOTE: spec calls for this actually in ios_base, but for now putting it
    // here so that it can reach into streambuf to grab it.  A slight but notable
    // deviation from standard C++
    locale_type getloc() const
    {
        return locale_provider_type::value();
    }

    char_type widen(char c) const
    {
        return use_facet<estd::ctype<char_type> >(getloc()).widen(c);
    }

    char narrow(char_type c, char /* default */)
    {
        return c;
    }
};


}}
