#pragma once

// just for IDE really - in production code, this file is always included by estd/ios.h
//#include "../ios.h"
#include "../type_traits.h"
#include "locale.h"
#include "../cstdint.h"
#include "../iosfwd.h"
#include "raw/variant.h"
#if ESTD_OS_TYPE
#include "../thread.h"
#endif
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

template <class Streambuf>
class basic_ios_base<Streambuf, true> : public ios_base
{
protected:
    typedef typename remove_reference<Streambuf>::type streambuf_type;

#if FEATURE_ESTD_STREAMBUF_TRAITS
    using signal_type = typename streambuf_type::traits_type::signal;
#endif

    streambuf_type* _rdbuf;

    streambuf_type* rdbuf() const { return _rdbuf; }

protected:
    EXPLICIT basic_ios_base(streambuf_type* sb) : _rdbuf(sb) {}

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
template <class Streambuf>
class basic_ios_base<Streambuf, false> : public ios_base
#if FEATURE_ESTD_STREAMBUF_TRAITS
    , remove_reference<Streambuf>::type::traits_type::signal
#endif
{
public:
    typedef typename remove_reference<Streambuf>::type streambuf_type;

protected:
#if FEATURE_ESTD_STREAMBUF_TRAITS
    using signal_type = typename streambuf_type::traits_type::signal;
    void init_signal()
    {
        rdbuf_.add_signal(this);
    }
#else
    void init_signal() {}
#endif
    Streambuf rdbuf_;

    // TODO: constructor needs cleanup here

#if __cpp_variadic_templates
    template <class ...TArgs>
    constexpr basic_ios_base(TArgs&&...args) :
        rdbuf_(std::forward<TArgs>(args)...)
    {
        init_signal();
    }

    constexpr  basic_ios_base(streambuf_type&& streambuf) :
        rdbuf_(std::move(streambuf))
    {
        init_signal();
    }
#else
    basic_ios_base() {}

    template <class TParam1>
    basic_ios_base(TParam1& p1) : rdbuf_(p1)
            {}
#endif
    basic_ios_base(streambuf_type& streambuf) :
        rdbuf_(streambuf) {}

#if FEATURE_ESTD_STREAMBUF_TRAITS
    ~basic_ios_base()
    {
        rdbuf_.del_signal(this);
    }
#endif

public:
    streambuf_type* rdbuf()
    { return &rdbuf_; }

#if FEATURE_ESTD_STREAMBUF_TRAITS
    signal_type& signal() { return *this; }
#endif
};


// See iosfwd for default values
template<ESTD_CPP_CONCEPT(concepts::v1::impl::StreambufBase) TStreambuf, bool use_pointer, class TPolicy>
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

    EXPLICIT operator bool() const { return base_type::fail() == false; }
};


}}
