#pragma once

#include "platform.h"

namespace estd { namespace internal {

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

}}