/**
 * @file
 */
#pragma once

#include "../type_traits.h"

namespace estd {

namespace experimental {

// kind of a blend of reference_wrapper and aligned_storage
// experimental, but very useful (abusing raw_instance_provider for this right now)
// NOTE: This in theory could be 1:1 with a fixed allocator, who should be using aligned_storage too
template <class T, size_t Align = alignof(T)>
class instance_wrapper
{
    // Not sure an item all by its lonesome in a class is gonna benefit from aligned_storage
    // perhaps instead we should make it a parent?
    typename estd::aligned_storage<sizeof(T), Align>::type _value;

public:
    typedef T value_type;

    value_type& value() { return *(T*)&_value; }
    const value_type& value() const { return *(T*)&_value; }

#if defined(FEATURE_CPP_VARIADIC) && defined(FEATURE_CPP_MOVESEMANTIC)
    template <class ...TArgs>
    value_type& construct(TArgs&&...args)
    {
        void* loc = reinterpret_cast<void*>(&_value);
        new (loc) value_type(std::forward<TArgs>(args)...);
        return value();
    }
#else
    value_type& construct()
    {
        new (&_value) value_type();
        return value();
    }

    template <class TParam1>
    value_type& construct(TParam1 p1)
    {
        new (&_value) value_type(p1);
        return value();
    }

    template <class TParam1, class TParam2>
    value_type& construct(TParam1 p1, TParam2 p2)
    {
        new (&_value) value_type(p1, p2);
        return value();
    }
#endif

    void destroy()
    {
        value().~value_type();
    }

    operator value_type& () { return value(); }

    operator const value_type& () const { return value(); }

    value_type& operator =(const value_type& copy_from)
    {
        value() = copy_from;
        return value();
    }
};


}

}
