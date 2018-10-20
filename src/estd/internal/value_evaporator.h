#pragma once

#include "platform.h"
#include "../utility.h"
#include "../type_traits.h"

namespace estd { namespace internal {

// utility class using specialization to eliminate a stored value from memory if the consuming
// class does not require it and can instead always rely on a default value
// NOTE: beware, https://stackoverflow.com/questions/5687540/non-type-template-parameters
// and my own findings indicate TEvaporated can't be a class/struct value
// FIX: Looking like is_present logic here is backwards
template <class TValue, bool is_present, class TEvaporated = TValue, TEvaporated default_value = TEvaporated()>
class value_evaporator;

template <class TValue, class TEvaporated, TEvaporated default_value>
class value_evaporator<TValue, false, TEvaporated, default_value>
{
protected:
    void value(TEvaporated) {}

    // NOTE: special version for when TValue is a ref
    value_evaporator(TEvaporated, bool) {}

public:
    TEvaporated value() const { return default_value; }

    static CONSTEXPR bool is_evaporated = true;

    value_evaporator(TEvaporated) {}
    value_evaporator() {}
};

template <class TValue, class TEvaporated, TEvaporated default_value>
class value_evaporator<TValue, true, TEvaporated, default_value>
{
protected:
    TValue m_value;

    void value(const TValue& v) { m_value = v; }

    // NOTE: special version for when TValue is a ref
    value_evaporator(TValue v, bool) : m_value(v) {}

public:
    TValue value() const { return m_value; }

    static CONSTEXPR bool is_evaporated = false;

    value_evaporator(const TValue& v) : m_value(v) {}
    value_evaporator() {}
};


template <class T, bool is_present>
struct reference_evaporator;

template <class T>
struct reference_evaporator<T, false>
{
    typedef typename estd::remove_reference<T>::type value_type;
    typedef T& reference;
    // FIX: Get proper name for this, this will be a value or ref depending on how things
    // got evaporated
    typedef value_type ref_type_exp;
    typedef value_type const_ref_type_exp;

    value_type value() { return value_type{}; }
    const value_type value() const { return value_type{}; }

    reference_evaporator(reference) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_evaporator(value_type&&) {}
#endif
};


template <class T>
struct reference_evaporator<T, true>
{
    typedef typename estd::remove_reference<T>::type value_type;
    typedef T& reference;
    typedef const T& const_reference;
    // FIX: Get proper name for this, this will be a value or ref depending on how things
    // got evaporated
    typedef reference ref_type_exp;
    typedef const_reference const_ref_type_exp;

    reference m_value;

    reference value() { return m_value; }
    const_reference value() const { return m_value; }

    reference_evaporator(reference value) : m_value(value) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    reference_evaporator(value_type&& value) :
    //m_value(std::move(value))
            m_value(value)
    {}
#endif
};

}

// very similar to value evaporator but this is for scenarios where we explicitly know
// exactly what kind of evaporation we are after
// experimental because I am mostly, but not completely, sure that this is sound and useful
namespace experimental {

template <class T, T v>
struct global_provider
{
    typedef typename estd::remove_reference<T>::type  value_type;

    static value_type& value() { return v; }
    static void value(const value_type& _v) { v = _v; }

#ifdef FEATURE_CPP_MOVESEMANTIC
    static void value(value_type&& _v) { v = std::move(_v); }
#endif
};

// FIX: reconcile this with global_provider
// NOTE: Including const in here irritates esp32 compiler, and doesn't
// appear to help anyone else
template <class T, T* /* const */ v>
struct global_pointer_provider
{
    typedef T* /* const */ value_type;

    static value_type value() { return v; }
};

template <class T, T v>
struct literal_provider : global_provider<T, v>
{
    static const T value() { return v; }

};


template <class T>
struct instance_provider
{
    typedef typename estd::remove_reference<T>::type  value_type;

    T _value;

    T& value() { return _value; }
    const T& value() const { return _value; }

    void value(const T& v) { _value = v; }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void value(T&& v) { _value = std::move(v); }
#endif

//protected:
    instance_provider(const T& v) : _value(v) {}

#ifdef FEATURE_CPP_MOVESEMANTIC
    instance_provider(T&& v) : _value(std::move(v)) {}
#endif

#ifdef FEATURE_CPP_VARIADIC
    // basically an emplace type operation
    template <class ...TArgs>
    instance_provider(TArgs...args) :
        _value(std::forward<TArgs>(args)...)
    {}
#endif
};


// FIX: pretty sure std C++ has a thing akin to this
template <class T, std::ptrdiff_t size = sizeof(T)>
struct raw_instance_provider
{
#ifdef BROKEN_FEATURE_CPP_ALIGN
    typename estd::aligned_storage<size, alignof (T)>::type buf;
#else
    // NOTE: Watch out for alignment issues here
    estd::byte buf[size];
#endif

    T& value() { return *reinterpret_cast<T*>(buf); }
    const T& value() const { return *reinterpret_cast<const T*>(buf); }
};

// tracks as a pointer, but presents as an inline instance/reference
template <class T>
struct instance_from_pointer_provider
{
    typedef typename estd::remove_reference<T>::type  value_type;

    T* _value;

    T& value() { return *_value; }
    const T& value() const { return *_value; }
    void value_ptr(T* v) { _value = v; }
    T* value_ptr() const { return _value; }

    instance_from_pointer_provider(T* v) : _value(v) {}
};


// semi-converts value to pointer on value() call
template <class T>
struct pointer_from_instance_provider
{
    typedef typename estd::remove_reference<T>::type  value_type;

    T _value;

    T* value() { return &_value; }
    const T* value() const { return &_value; }

    pointer_from_instance_provider(const T& v) : _value(v) {}

protected:
};


template <class T, T temporary_value = T()>
struct temporary_provider
{
    typedef T  value_type;

    static T value() { return temporary_value; }
};

}

}
