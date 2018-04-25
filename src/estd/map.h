#pragma once

#include "utility.h"
#include "functional.h"
#include "array.h"

// so far not liking this mapping of map
// prefer the mc-memory-lib approach

// Use this to be less C++ std compliant (use ptr instead of ref) but fit
// slightly more comfortably into embedded world.  Disabling this shall require
// a "null value" to be floating somewhere to check against
#define FEATURE_ESTDLIB_MAPPTR

namespace estd {

template <class Key, class T, class Compare = estd::less<Key> >
class map_base
{
public:
};

namespace layer1 {

// In the short term, no sorting actually is done or expected.  In the longer term we'll
// definitely want that.  Consider a trait which indicates whether array is pre-sorted
template <class Key, class T, std::size_t size, class Compare = estd::less<Key> >
class map : public map_base<Key, T, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef map_base<Key, T, Compare> base_t;
    //typedef typename base_t::value_type value_type;

public:
    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

private:
    array<value_type, size> _map;

#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    map() {}

    // FIX: Pretty sure this only will ever copy, and we need really an initializer_list
    // type behavior.  However, in lieu of aforementioned this is needed for unit tests
    map(array<value_type, size>& _map) : _map(_map) {}

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : _map)
        {
            if(i.first == key)
#ifdef FEATURE_ESTDLIB_MAPPTR
                return &i.second;
#else
                return i.second;
#endif
        }

        return NULLPTR;
    }
};

}

namespace layer2 {

// In the short term, no sorting actually is done or expected.  In the longer term we'll
// definitely want that.  Consider a trait which indicates whether array is pre-sorted
template <class Key, class T, std::size_t size, class Compare = estd::less<Key> >
class map : public map_base<Key, T, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef map_base<Key, T, Compare> base_t;

public:
    //typedef typename base_t::value_type value_type;

    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

private:
    layer2::array<value_type, size> _map;

#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    template <class TArray>
    map(TArray a) : _map(a) {}

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : _map)
        {
            if(i.first == key)
#ifdef FEATURE_ESTDLIB_MAPPTR
                return &i.second;
#else
                return i.second;
#endif
        }

        return NULLPTR;
    }
};

}


namespace layer3 {

// In the short term, no sorting actually is done or expected.  In the longer term we'll
// definitely want that.  Consider a trait which indicates whether array is pre-sorted
template <class Key, class T, class Compare = estd::less<Key> >
class map : public map_base<Key, T, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef map_base<Key, T, Compare> base_t;

public:
    //typedef typename base_t::value_type value_type;

    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

private:
    layer3::array<value_type> _map;

#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    //template <class TArray>
    //map(TArray a) : _map(a) {}

    template <size_t N>
    map(value_type (&array) [N]) : _map(array)
    {
        //base_t::m_array = array;
    }

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : _map)
        {
            if(i.first == key)
#ifdef FEATURE_ESTDLIB_MAPPTR
                return &i.second;
#else
                return i.second;
#endif
        }

        return NULLPTR;
    }
};
}


}
