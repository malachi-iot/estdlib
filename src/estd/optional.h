/**
 *  @file
 */
#pragma once

#if __cpp_exceptions
#include <exception>
#endif

#include "utility.h"
#include "memory.h"

#include "internal/feature/optional.h"
#include "internal/functional/hash.h"
#include "internal/optional.h"

namespace estd {

struct nullopt_t {
#ifdef __cpp_constexpr
    explicit constexpr
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

#if __cpp_exceptions
class bad_optional_access : public std::exception {};
#endif

// with some guidance from https://www.bfilipek.com/2018/05/using-optional.html#intro
template <class T, class Base>
class optional :
        public internal::optional_tag_base,
        public Base
{
    using base_type = Base;

#if __cpp_rvalue_references
    /// if current value exists and is not trivial, destroy it
    /// then do an assign or direct initialize, depending on
    /// what T prefers
    template <class U>
    void assign_value(U&& u)
    {
        base_type::assign_value(
            base_type::has_value(),
            std::forward<U>(u));
        base_type::has_value(true);
    }
#else
    template <class U>
    void assign_value(const U& u)
    {
        base_type::assign_value(base_type::has_value(), u);
        base_type::has_value(true);
    }
#endif

public:
    typedef typename base_type::value_type value_type;

protected:
    // Determines if we're returning a true reference to underlying value (std optional
    // behavior) or a value/copy (such as with bitwise flavor)
    static CONSTEXPR bool real_reference = base_type::real_reference;
        // DEBT: Seemed to be working, but now it doesn't
        //is_same<add_rvalue_reference<value_type>, return_type>::value;

    typedef internal::value_type_morph<T, real_reference ?
        internal::value_type_morphs::none :
        internal::value_type_morphs::no_reference> type_morph;

    void assert_has_value() const
    {
#if __cpp_exceptions
        if(!base_type::has_value()) throw bad_optional_access();
#else
        if(!base_type::has_value()) abort();
#endif
    }

public:
    // --- constructors
    // We depend on base class to initialize default to has_value() == false
    ESTD_CPP_DEFAULT_CTOR(optional)

    ESTD_CPP_CONSTEXPR_RET optional(nullopt_t) {}

#ifdef __cpp_rvalue_references
    template < class U, class TUBase >
    constexpr optional(optional<U, TUBase>&& move_from) :
        base_type(std::move(move_from))
    {
    }

    // DEBT: I was hoping default of this would cascade down to the
    // user defined one for optional_base but so far no dice
    constexpr optional(optional&& move_from) :
        base_type(std::move(move_from))
    {
    }

    /**
     * DEBT: In the spec, c++17 utilizes deduction guides here.
     * So should we
     **/
    template <class U, class enabled = enable_if_t<
        is_constructible<T, U&&>::value &&
        is_same<remove_cvref_t<U>, in_place_t>::value == false &&
        is_base_of<optional_tag_base, remove_cvref_t<U>>::value == false
        > >
    constexpr optional(U&& value) : base_type(in_place_t{}, std::forward<U>(value))
    {
    }
#else
    optional(const value_type& copy_from) :
        base_type(in_place_t(), copy_from) {}
#endif

    // DEBT: I was hoping default of this would cascade down to the
    // user defined one for optional_base but so far no dice
    ESTD_CPP_CONSTEXPR_RET optional(const optional& copy_from) :
        base_type(copy_from)
    {
    }

#if __cpp_variadic_templates
    template <class ...TArgs>
    constexpr explicit optional(in_place_t, TArgs&&...args) :
        base_type(in_place_t{}, std::forward<TArgs>(args)...)
    {}
#else
    template <class T1>
    optional(in_place_t, const T1& v1) :
        base_type(in_place_t(), v1)
    {}
#endif

    template <class U, class TUBase>
    optional(const optional<U, TUBase>& copy_from) :
        base_type(copy_from)
    {
    }

    // --- assignment operators

    optional& operator=(nullopt_t)
    {
        base_type::reset();
        return *this;
    }


    // DEBT: Still need 'move' variety
    template <class U, class TBase2
#if __cplusplus >= 201103L
        , class enabled = typename enable_if<
            is_constructible<T, optional<U>&>::value == false &&
            is_convertible<T, optional<U>&>::value == false &&
            is_assignable<T, optional<U>&>::value == false>::type
#endif
        >
    optional& operator=(const optional<U, TBase2>& assign_from)
    {
        if(assign_from.has_value())
            assign_value(assign_from.value());
        else
            base_type::reset();

        return *this;
    }

    optional& operator=(const optional& assign_from)
    {
        if(assign_from.has_value())
            assign_value(assign_from.value());
        else
            base_type::reset();

        return *this;
    }

#ifdef __cpp_rvalue_references
    template <class U, class TBase2, class enabled =
        typename enable_if<
            is_constructible<T, optional<U>&>::value == false &&
            is_convertible<T, optional<U>&>::value == false &&
            is_assignable<T, optional<U>&>::value == false>::type
        >
    optional& operator=(optional<U, TBase2>&& move_from)
    {
        if(move_from.has_value())
            assign_value(std::move(move_from.value()));
        else
            base_type::reset();

        return *this;
    }


    template<class U = T, class enabled = enable_if_t<
        is_assignable<T&, U>::value &&
        is_constructible<T, U>::value &&
        is_base_of<optional_tag_base, remove_cvref_t<U>>::value == false> >
    optional& operator=(U&& v)
    {
        assign_value(std::forward<U>(v));

        return *this;
    }
#else
    optional& operator=(const value_type& v)
    {
        // DEBT: Do same has_value interrogation as c++11 version above
        assign_value(v);
        return *this;
    }
#endif


#ifdef __cpp_variadic_templates
    // NOTE: This one won't work with bitwise version.  In theory
    // it could work with layer1 version, but doesn't currently.
    // In both cases, it's better to use assignment operator when
    // possible since they are trivial
    template<class... TArgs>
    T& emplace(TArgs&&... args)
    {
        base_type::destroy();

        T& v = base_type::emplace(std::forward<TArgs>(args)...);

        base_type::has_value(true);

        return v;
    }
#endif

    // -- getters

#if __cpp_rvalue_references
    typename type_morph::reference value() &
    {
        return (assert_has_value(), base_type::value());
    }

    constexpr typename type_morph::const_reference value() const&
    {
        return (assert_has_value(), base_type::value());
    }

    typename type_morph::rvalue_reference value() &&
    {
        return (assert_has_value(), std::forward<value_type>(base_type::value()));
    }

    constexpr typename type_morph::const_rvalue_reference value() const &&
    {
        return (assert_has_value(),
            std::forward<const value_type>(base_type::value()));
    }
#else
    typename type_morph::reference value()
    {
        return (assert_has_value(), base_type::value());
    }

    typename type_morph::const_reference value() const
    {
        return (assert_has_value(), base_type::value());
    }
#endif

#ifdef __cpp_rvalue_references
    template <class U>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
    constexpr
#endif
    T value_or(U&& default_value) &&
    {
        return base_type::has_value() ? base_type::value() : std::forward<U>(default_value);
    }

    template <class U>
    constexpr T value_or(U&& default_value) const&
    {
        return base_type::has_value() ? base_type::value() : std::forward<U>(default_value);
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

    typename type_morph::reference operator*() { return base_type::value(); }
    typename type_morph::const_reference operator*() const { return base_type::value(); }
};


namespace layer1 {

#if FEATURE_ESTD_OPTIONAL_LAYER1_ALIAS
template <class T, T null_value = internal::optional_default_value<T>::value>
using optional = estd::optional<T, internal::layer1::optional<T, null_value> >;
#else
// DEBT: Has some kind of MSVC incompatibility
template <class T, T null_value = internal::optional_default_value<T>::value>
class optional : public estd::optional<T, internal::layer1::optional<T, null_value> >
{
    typedef estd::optional<T, internal::layer1::optional<T, null_value> > base_type;
    typedef typename base_type::value_type value_type;

protected:
    /*
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
    } */

public:
    ESTD_CPP_DEFAULT_CTOR(optional)

#ifdef __cpp_rvalue_references
    // Not doing 'class U' because layer1 is expected to be trivial
    optional(value_type&& v) : base_type(in_place_t{}, std::forward<value_type>(v))
    { }
#else
    optional(const value_type& copy_from) : base_type(copy_from)
    { }
#endif

    template < class U, class TUBase >
    optional( const estd::optional<U, TUBase>& copy_from ) :
        base_type(copy_from)
    {
        //copy(copy_from);
    }

    ESTD_CPP_CONSTEXPR_RET optional(estd::nullopt_t no) : base_type(no) {}

    optional& operator=(estd::nullopt_t)
    {
        base_type::reset();
        return *this;
    }

#ifdef __cpp_rvalue_references
    //template <class U = T>
    //optional& operator=(U&& value)
    optional& operator=(value_type&& value)
    {
        base_type::operator=(std::forward<value_type>(value));
        return *this;
    }
#endif

    // DEBT: Spec doesn't have this, I don't think lvalues are
    // supposed to be supported
    optional& operator=(const value_type& value)
    {
        base_type::operator=(value);
        return *this;
    }
};
#endif

}

template <class T, class U, class Base>
constexpr bool operator==(const optional<T, Base>& opt, const U& value)
{
    return opt.has_value() ? *opt == value : false;
}


template <class T, class U, class Base>
constexpr bool operator==(const U& value, const optional<T, Base>& opt)
{
    return opt.has_value() ? value == *opt : false;
}

template <class T, class T2, class Base, class Base2>
constexpr bool operator==(const optional<T, Base>& lhs, const optional<T2, Base2>& rhs)
{
    return lhs.has_value() && rhs.has_value() ?
        *lhs == *rhs :
        !lhs.has_value() && !rhs.has_value();
}

template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator!=(const optional<T, TBase>& opt, const U& value)
{
    return opt.has_value() ? *opt != value : true;
}


template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator>(const optional<T, TBase>& opt, const U& value)
{
    return opt && *opt > value;
}


template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator<(const optional<T, TBase>& opt, const U& value)
{
    return opt && *opt < value;
}

template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator<=(const optional<T, TBase>& opt, const U& value)
{
    return opt && *opt <= value;
}

template <class T, class U, class TBase>
ESTD_CPP_CONSTEXPR_RET bool operator>=(const optional<T, TBase>& opt, const U& value)
{
    return opt && *opt >= value;
}

template <class T, class Base>
struct hash<optional<T, Base>>
{
    constexpr size_t operator()(const optional<T, Base>& v) const
    {
        using underlying = hash<estd::remove_const_t<T>>;

        return underlying{}(*v);
    }
};

}
