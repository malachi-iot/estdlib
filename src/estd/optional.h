/**
 *  @file
 */
#pragma once

#include "utility.h"
#include "memory.h"

#include "internal/optional.h"

namespace estd {

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


// DEBT: Pretty sure this won't work with c++03.  If it does, document that
#ifdef FEATURE_CPP_INLINE_VARIABLES
inline
#elif defined(FEATURE_CPP_INLINE_STATIC)
static
#endif
#ifdef FEATURE_CPP_INITIALIZER_LIST
CONSTEXPR nullopt_t nullopt{0};
#else
CONSTEXPR nullopt_t nullopt(0);
#endif

// Non-standard workaround for my own failing move semantic
// however, it will come in handy for pre move semantic compilations as well
//#define FEATURE_ESTD_OPTIONAL_LVALUE_ASSIGN

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
        base_type::value(copy_from.value());
    }

public:
    // even this doesn't help de-selecting this for value-initializer type
    // constructor
    //typedef void optional_tag;
    typedef typename base_type::value_type value_type;

/*
    //void value(value_type& v) { base_type::value(v); }
    value_type& value() { return base_type::value(); }
    const value_type& value() const { return base_type::value(); }
*/

    // --- constructors
    // We depend on base class to initialize default to has_value() == false
    ESTD_CPP_DEFAULT_CTOR(optional)

    ESTD_CPP_CONSTEXPR_RET optional(nullopt_t) {}

#ifdef __cpp_rvalue_references
    template < class U, class TUBase >
    optional(optional<U, TUBase>&& move_from )
    {
        base_type::has_value(move_from.has_value());
        new (&base_type::value()) value_type(std::move(move_from.value()));
    }

    /**
     * FIX: really do not like brute forcing the is_same here, but
     * no provision to detect (and reject) presence of 'optional'
     * class itself here was working.  In the spec, c++17 utilizes
     * deduction guides for this purpose
     **/
    template <class U = value_type
            ,
            //class = estd::enable_if_t<!estd::is_base_of<internal::optional_tag_base, U>::value >
            class = typename estd::enable_if_t<
                  //!has_optional_tag_typedef<U>::value &&
                    estd::is_same<U, value_type>::value
                    >
              >
    optional(U&& move_from)
    {
        base_type::has_value(true);
        new (&base_type::value()) value_type(std::move(move_from));
    }
#endif

    optional(const value_type& copy_from) : base_type(copy_from) {}

    template < class U, class TUBase >
    optional( const optional<U, TUBase>& copy_from )
    {
        copy(copy_from);
    }

    // --- assignment operators

    optional& operator=(nullopt_t)
    {
        base_type::reset();
        return *this;
    }


    template <class U, class TBase2>
    optional& operator=(const optional<U, TBase2>& assign_from)
    {
        copy(assign_from);
        return *this;
    }

#ifdef __cpp_rvalue_references
    // DEBT: disabling the template part of this because it's
    // getting too greedy and consuming other 'optional' , then
    // that results in the incorrect 'operator bool' cast
    //template< class U = T >
    //optional& operator=( U&& v )
    optional& operator=(value_type&& v)
    {
        base_type::value(std::move(v));
        base_type::has_value(true);
        return *this;
    }
#endif

#ifdef FEATURE_ESTD_OPTIONAL_LVALUE_ASSIGN
    // FIX: spec doesn't have this, but I think my lack of class U = T
    // may be breaking things so stuffing this in here, for now
    optional& operator=(const value_type& v)
    {
        new (&base_type::value()) value_type(v);
        base_type::has_value(true);
        return *this;
    }
#endif


#ifdef __cpp_variadic_templates
    // NOTE: This one won't work with bitwise version
    template< class... TArgs >
    T& emplace( TArgs&&... args )
    {
        T& v = base_type::value();
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
        return base_type::has_value() ? base_type::value() : std::move(default_value);
    }

    template <class U>
    constexpr T value_or( U&& default_value ) const&
    {
        return base_type::has_value() ? base_type::value() : std::move(default_value);
    }
#else
    // Untested
    template <class U>
    const T& value_or(const U& default_value) const
    {
        return base_type::has_value() ? base_type::value() : default_value;
    }
#endif

#if __cpp_constexpr
    constexpr explicit
#endif
    operator bool() const { return base_type::has_value(); }

    typename base_type::return_type operator*() { return base_type::value(); }
    typename base_type::const_return_type operator*() const { return base_type::value(); }
};


namespace layer1 {

template <class T, T null_value = T()>
class optional : public estd::optional<T, internal::layer1::optional_base<T, null_value> >
{
    typedef estd::optional<T, internal::layer1::optional_base<T, null_value> > base_type;
    typedef typename base_type::value_type value_type;

protected:
    template < class U, class TUBase >
    void copy(const estd::optional<U, TUBase>& copy_from)
    {
        if(copy_from.has_value())
        {
            // if has_value is true but this is null_value, we have a runtime error
            if(copy_from.value() == null_value)
            {
                // TODO: assert that copy_from has_value value aligns with incoming value() itself
                // note that this has to be a runtime assertion
            }
            new (&base_type::value()) value_type(copy_from.value());
        }
        else
        {
            new (&base_type::value()) value_type(null_value);
        }
    }

public:
    optional() {}

#ifdef __cpp_rvalue_references
    // FIX: Not doing 'U' deduction for same reason that
    // we can't properly detect presence of 'optional' coming
    // in here
    //template <class U>
    optional(value_type&& v) : base_type(std::move(v))
    { }
#endif

    optional(const value_type& copy_from) : base_type(copy_from)
    { }

    template < class U, class TUBase >
    optional( const estd::optional<U, TUBase>& copy_from )
    {
        copy(copy_from);
    }

    optional(estd::nullopt_t no) : base_type(no) {}

    optional& operator=(estd::nullopt_t)
    {
        base_type::value(null_value);
        return *this;
    }

#ifdef __cpp_rvalue_references
    //template <class U = T>
    //optional& operator=(U&& value)
    optional& operator=(value_type&& value)
    {
        base_type::operator=(std::move(value));
        return *this;
    }
#endif

#ifdef FEATURE_ESTD_OPTIONAL_LVALUE_ASSIGN
    // FIX: spec doesn't have this, but I think my lack of class U = T
    // may be breaking things so stuffing this in here, for now
    optional& operator=(const value_type& value)
    {
        base_type::operator=(value);
        return *this;
    }
#endif

/*
    optional& operator=(const estd::optional<value_type>& assign_from)
    {
        copy(assign_from);
        return *this;
    } */

#if __cpp_constexpr
    constexpr explicit
#endif
    operator bool() const { return base_type::has_value(); }
};


template <>
class optional<bool> : public estd::optional<bool, estd::internal::optional_bitwise<bool, 1> >
{
    typedef estd::optional<bool, estd::internal::optional_bitwise<bool, 1> > base_type;

public:

    ESTD_CPP_FORWARDING_CTOR(optional)
};


}

template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator==(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? opt.value() == value : false;
}


template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator==(const U& value, const optional<T, TBase>& opt)
{
    return opt.has_value() ? value == opt.value() : false;
}


template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator!=(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? opt.value() != value : true;
}


template <class T, class U, class TBase>
CONSTEXPR bool operator>(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? opt.value() > value : false;
}


template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator<(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? opt.value() < value : false;
}



}
