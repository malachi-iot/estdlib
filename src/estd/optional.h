/**
 *  @file
 */
#pragma once

#include "utility.h"
#include "memory.h"

namespace estd {

namespace internal {

struct optional_tag_base
{
    typedef void optional_tag;

    ESTD_FN_HAS_TYPEDEF_EXP(optional_tag)
};

struct optional_base_base
{
    bool m_initialized;

    optional_base_base(bool initialized = false) :
        m_initialized(initialized) {}

    bool has_value() const { return m_initialized; }

protected:
    void has_value(bool initialized) { m_initialized = initialized; }
};

template <class T>
struct optional_base : optional_base_base
{
    typedef T value_type;

    optional_base(bool initialized = false) : optional_base_base(initialized) {}

    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front
    experimental::raw_instance_provider<T> provider;

    value_type& value() { return provider.value(); }
    const value_type& value() const { return provider.value(); }
};


// experimental: use this when we don't need an external flag, but instead some
// specific value of T represents a NULL/not present.  To really work well this
// needs specialization on T, which might not be easy since typedef's fall back
// down to int, short, long, etc.
template <class T, T null_value>
struct optional_reserved_base
{
    //typename aligned_storage<sizeof(T), alignof (T)>::type storage;
    // TODO: will need attention on the alignment front
    experimental::raw_instance_provider<T> provider;

    bool has_value() const { return provider.value() == null_value; }

protected:
    // FIX: this is a noop, but that is kind of clumsy in cases where
    // it's passed in a false - except that currently never happens,
    // therefore there's potential to clean this up
    void has_value(bool) {}
};

}

struct nullopt_t {
    explicit
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    nullopt_t(int) {}

#if !defined(FEATURE_CPP_INLINE_VARIABLES) && !defined(FEATURE_CPP_INLINE_STATIC)
    /// \brief in the case where we can't easily make a global 'nullopt',
    /// make a provision for more easily creating a nullopt_t on the fly
    ///
    nullopt_t() {}
#endif
};


#ifdef FEATURE_CPP_INLINE_VARIABLES
inline
#elif defined(FEATURE_CPP_INLINE_STATIC)
static
#endif
CONSTEXPR nullopt_t nullopt{0};

// with some guidance from https://www.bfilipek.com/2018/05/using-optional.html#intro
template <class T, class TBase = internal::optional_base<T> >
class optional :
        public internal::optional_tag_base,
        public TBase
{
    typedef TBase base_type;

protected:
    template < class U, class TUBase >
    void copy(const optional<U, TUBase>& copy_from)
    {
        base_type::has_value(copy_from.has_value());
        if(copy_from.has_value())
        {
            new (&base_type::value()) value_type(copy_from.value());
        }
    }

public:
    // even this doesn't help de-selecting this for value-initializer type
    // constructor
    //typedef void optional_tag;
    typedef typename base_type::value_type value_type;

    //void value(value_type& v) { base_type::value(v); }
    value_type& value() { return base_type::value(); }
    const value_type& value() const { return base_type::value(); }

    // --- constructors

    optional() {}

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    optional(nullopt_t) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    template < class U, class TUBase >
    optional(optional<U, TUBase>&& move_from )
    {
        base_type::has_value(move_from.has_value());
        new (&value()) value_type(std::move(move_from.value()));
    }

    /**
     * Just won't work.  In every case layer1 gets passed into here
    template <class U = value_type
            ,
            //class = estd::enable_if_t<!estd::is_base_of<internal::optional_tag_base, U>::value >
            class = typename estd::enable_if<!has_optional_tag_typedef<U>::value>::type
                    >
    optional(U&& move_from)
    {
        base_type::has_value(true);
        new (&value()) value_type(std::move(move_from));
    }
     */
#endif

    template < class U, class TUBase >
    optional( const optional<U, TUBase>& copy_from )
    {
        copy(copy_from);
    }

    // --- assignment operators

    optional& operator=(nullopt_t)
    {
        base_type::has_value(false);
        return *this;
    }


    template <class U, class TBase2>
    optional& operator=(const optional<U, TBase2>& assign_from)
    {
        copy(assign_from);
        return *this;
    }

#ifdef FEATURE_CPP_MOVESEMANTIC
    // FIX: disabling the template part of this because it's
    // getting too greedy and consuming other 'optional' , then
    // that results in the incorrect 'operator bool' cast
    //template< class U = T >
    //optional& operator=( U&& v )
    optional& operator=(value_type&& v)
    {
        // FIX: ends up treating 'v' as a bool using bool operator
        new (&value()) value_type(std::move(v));
        base_type::has_value(true);
        return *this;
    }
#else
    // Untested
    optional& operator=(const value_type& v)
    {
        new (&value()) value_type(v);
        base_type::has_value(true);
        return *this;
    }

#endif

#ifdef FEATURE_CPP_VARIADIC
    template< class... TArgs >
    T& emplace( TArgs&&... args )
    {
        T& v = value();
        new (&v) T(std::forward<TArgs>(args)...);
        return v;
    }
#endif

#ifdef FEATURE_CPP_MOVESEMANTIC
    template< class U >
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    constexpr
#endif
    T value_or( U&& default_value ) &&
    {
        return base_type::has_value() ? value() : std::move(default_value);
    }

    template <class U>
    constexpr T value_or( U&& default_value ) const&
    {
        return base_type::has_value() ? value() : std::move(default_value);
    }
#else
    // Untested
    template <class U>
    const T& value_or(const U& default_value) const
    {
        return base_type::has_value() ? value() : default_value;
    }
#endif

    operator bool() const { return base_type::has_value(); }

    value_type& operator*() { return value(); }
    const value_type& operator*() const { return value(); }
    value_type* operator->() { return &value(); }
    const value_type* operator->() const { return &value(); }
};


// layer1 version considered experimental
namespace layer1 {

namespace internal {

template <class T, T null_value>
class optional_base //: public estd::internal::optional_tag_base
{
    T _value;

protected:
//public:
    //optional_base(T& value) : _value(value) {}
    // should always bool == true here
    optional_base(bool) {}

    optional_base() : _value(null_value) {}

    void value(T& value)
    {
        _value = value;
    }

public:
    typedef T value_type;

    bool has_value() const { return _value != null_value; }
    void has_value(bool) {}
    value_type& value() { return _value; }
    const value_type& value() const { return _value; }
};

}

template <class T, T null_value = T()>
class optional : public estd::optional<T, internal::optional_base<T, null_value> >
{
    typedef estd::optional<T, internal::optional_base<T, null_value> > base_type;
    typedef typename base_type::value_type value_type;

public:
    optional() {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    //optional(value_type&& v) : base_type(std::move(v))
    //{ }

    template < class U, class TUBase >
    optional( const estd::optional<U, TUBase>& copy_from )
        //: base_type(copy_from)
    {
        // TODO: assert that copy_from has_value value aligns with incoming value() itself
        // note that this has to be a runtime assertion
        //value_type& v = copy_from.value();
        //base_type::value(v);
        new (&base_type::value()) value_type(copy_from.value());
    }
#else
    optional(value_type& copy_from) : base_type(copy_from)
    { }
#endif

    optional(estd::nullopt_t) {}

    optional& operator=(estd::nullopt_t)
    {
        new (&base_type::value()) value_type(null_value);
        return *this;
    }

    //template <class U = T>
    //optional& operator=(U&& value)
    optional& operator=(value_type&& value)
    {
        base_type::operator=(std::move(value));
        return *this;
    }

    optional& operator=(estd::optional<value_type>& assign_from)
    {
        base_type::operator=(assign_from);
        return *this;
    }

};

}

}
