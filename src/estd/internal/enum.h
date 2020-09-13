/*
 * @file
 */
#pragma once

namespace estd { namespace internal {

// To work around < C++11's lack of "enum class"
// TEnumClass must always contain an enum called 'values'
template <class TEnumClass, class TValue = int>
struct enum_class : TEnumClass
{
protected:
    TValue value;

public:
    explicit enum_class(TValue value) : value(value) {}

    typedef typename TEnumClass::values values;

    enum_class(values value) : value(value) {}

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
};

}}