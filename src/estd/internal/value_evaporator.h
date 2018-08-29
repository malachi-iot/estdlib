#pragma once

#include "platform.h"
#include "../utility.h"

namespace estd { namespace internal {

// utility class using specialization to eliminate a stored value from memory if the consuming
// class does not require it and can instead always rely on a default value
template <class TValue, bool is_present, class TEvaporated = TValue, TEvaporated default_value = TEvaporated()>
class value_evaporator;

template <class TValue, class TEvaporated, TEvaporated default_value>
class value_evaporator<TValue, true, TEvaporated, default_value>
{
protected:
    void value(TEvaporated) {}

public:
    TEvaporated value() const { return default_value; }

    static CONSTEXPR bool is_evaporated = true;

    value_evaporator(TEvaporated) {}
    value_evaporator() {}
};

template <class TValue, class TEvaporated, TEvaporated default_value>
class value_evaporator<TValue, false, TEvaporated, default_value>
{
protected:
    TValue m_value;

    void value(const TValue& v) { m_value = v; }

public:
    TValue value() const { return m_value; }

    static CONSTEXPR bool is_evaporated = false;

    value_evaporator(const TValue& v) : m_value(v) {}
    value_evaporator() {}
};


}

// very similar to value evaporator but this is for scenarios where we explicitly know
// exactly what kind of evaporation we are after
// experimental because I am mostly, but not completely, sure that this is sound and useful
namespace experimental {

template <class T, T& v>
struct global_provider
{
    static T& value() { return v; }
    static void value(const T& _v) { v = _v; }

#ifdef FEATURE_CPP_MOVESEMANTIC
    static void value(T&& _v) { v = std::move(_v); }
#endif
};


template <class T>
struct instance_provider
{
    T _value;

    T& value() { return _value; }
    const T& value() const { return _value; }

    void value(const T& v) { _value = v; }

#ifdef FEATURE_CPP_MOVESEMANTIC
    void value(T&& v) { _value = std::move(v); }
#endif

protected:
    instance_provider(const T& v) : _value(v) {}
#ifdef FEATURE_CPP_MOVESEMANTIC
    instance_provider(T&& v) : _value(std::move(v)) {}
#endif
};


template <class T>
struct pointer_from_value_provider
{
    T _value;

    T* value() { return &_value; }
    const T* value() const { return &_value; }

protected:
};


template <class T, T temporary_value = T()>
struct temporary_provider
{
    static T value() { return temporary_value; }
};

}

}
