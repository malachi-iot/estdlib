#pragma once

#include "utility.h"
#include "functional.h"
#include "array.h"
#include "vector.h"

// so far not liking this mapping of map
// prefer the mc-memory-lib approach

// Use this to be less C++ std compliant (use ptr instead of ref) but fit
// slightly more comfortably into embedded world.  Disabling this shall require
// a "null value" to be floating somewhere to check against
#define FEATURE_ESTDLIB_MAPPTR

namespace estd {

namespace experimental {

template <class TPair>
struct kvp_traits
{
    typedef typename TPair::first_type key_type;

    static CONSTEXPR key_type invalid_key() { return key_type(); }
};


template <class TValue>
struct kvp_traits<estd::pair<const int, TValue>>
{
    static CONSTEXPR const int invalid_key() { return -1; }
};

}

namespace internal {

template <
          class TCollection,
          class Compare = estd::less<typename TCollection::value_type::first_type>
          >
class map_base
{
protected:
    TCollection m_map;

    map_base(const TCollection& m_map) : m_map(m_map) {}

    typedef typename TCollection::value_type value_type;
    typedef typename value_type::first_type key_type;
    typedef typename value_type::second_type mapped_type;

    typedef experimental::kvp_traits<value_type> kvp_traits;

#ifdef FEATURE_CPP_INITIALIZER_LIST
    typedef ::std::initializer_list<value_type> initializer_list;

    map_base(initializer_list init,
        bool presorted = true,
        const Compare& comp = Compare()) :
        m_map(init)
    {
        if(!presorted)
        {
            // TODO: issue manual runtime sort here
        }
    }
#endif

public:
};

}



template <class Key, class T,
          class Compare = estd::less<Key>,
          template <class> class Allocator = std::allocator>
class map : public internal::map_base<estd::vector<pair<const Key, T>>, Compare>
{
protected:
    typedef internal::map_base<estd::vector<pair<const Key, T>>, Compare> base_t;

public:
    typedef Key key_type;
    typedef T mapped_type;

    typedef std::pair<const Key, T> value_type;

    typedef Compare key_compare;
    typedef Allocator<value_type> allocator_type;

    map() {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    typedef ::std::initializer_list<value_type> initializer_list;

    // for embedded scenarios, we often initialize map with a constant and pre-sorted list
    // if the list is at least pre-sorted, keep this as true.  If this is false, initialization_list
    // incoming lists *should* be sorted at runtime as part of the copy/construct process
    map(initializer_list init,
        bool presorted = true,
        const Compare& comp = Compare(),
        const allocator_type& alloc = allocator_type()) :
        base_t(init, comp) {}
#endif
};

namespace layer1 {

// In the short term, no sorting actually is done or expected.  In the longer term we'll
// definitely want that.  Consider a trait which indicates whether array is pre-sorted
template <class Key, class T, std::size_t size, class Compare = estd::less<Key> >
class map : public internal::map_base<array<pair<Key, T>, size>, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef internal::map_base<array<pair<Key, T>, size>, Compare> base_t;
    //typedef typename base_t::value_type value_type;

public:
    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    //map() {}

    // FIX: Pretty sure this only will ever copy, and we need really an initializer_list
    // type behavior.  However, in lieu of aforementioned this is needed for unit tests
    map(array<value_type, size>& _map) : base_t(_map) {}

#ifdef FEATURE_CPP_INITIALIZER_LIST
    map(typename base_t::initializer_list list) : base_t(list) {}
#endif

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : base_t::m_map)
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
class map : public internal::map_base<layer2::array<pair<Key, T>, size>, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef internal::map_base<layer2::array<pair<Key, T>, size>, Compare> base_t;

public:
    //typedef typename base_t::value_type value_type;

    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

private:
#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    template <class TArray>
    map(TArray a) : base_t(a) {}

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : base_t::m_map)
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
class map : public internal::map_base<layer3::array<pair<Key, T>>, Compare>
{
    typedef Key key_type;
    typedef T mapped_type;
    typedef internal::map_base<layer3::array<pair<Key, T>>, Compare> base_t;

public:
    //typedef typename base_t::value_type value_type;

    // NOTE: not making this const Key just yet, since initiatialization of a layer1::map
    // is kinda tricky
    typedef pair<Key, T> value_type;

#ifdef FEATURE_ESTDLIB_MAPPTR
    typedef mapped_type* mapped_t;
#else
    typedef mapped_type& mapped_t;
#endif

public:
    //template <class TArray>
    //map(TArray a) : _map(a) {}

    template <size_t N>
    map(value_type (&array) [N]) : base_t(array)
    {
        //base_t::m_array = array;
    }

    // Deviates from spec - since this particular map can't grow, we need to detect
    // when an operator[] lookup fails, therefore
    mapped_t operator[] (const key_type& key)
    {
        for(value_type& i : base_t::m_map)
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
