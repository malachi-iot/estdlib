/*
 * @file
 */
#pragma once

#include "macro/c++/attr.h"
#include "macro/cpp.h"

namespace estd { namespace internal {

// 30JAN26 MB - Obsolete: c++11 required.  Keeping around since it's
// otherwise harmless and some other libraries may still depend on it
// Once 01MAR26 arrives, safe to remove

// To work around < C++11's lack of "enum class"
// TEnumClass must always contain an enum called 'values'

template <class TEnumClass, class TValue = int>
struct enum_class : TEnumClass
{
    typedef typename TEnumClass::values values;

private:
    values value;

public:
    ESTD_CPP_ATTR_DEPRECATED("Use real c++11 enum class instead")
    ESTD_CPP_DEFAULT_CTOR(enum_class)

    // DEBT: We need to do a safer typecast here
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    explicit enum_class(TValue value) : value((values)value) {}

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    enum_class(values value) : value(value) {}

    enum_class& operator=(values v)
    {
        value = v;
        return *this;
    }

    bool operator == (values compare_to) const
    {
        return value == compare_to;
    }

    bool operator == (TValue compare_to) const
    {
        return value == compare_to;
    }

    bool operator != (values compare_to) const
    {
        return value != compare_to;
    }

#if __cplusplus >= 201103L
    explicit
#endif    
    operator TValue () const
    {
        return static_cast<TValue>(value);
    }
};

}}
