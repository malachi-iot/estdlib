#pragma once

#include "../../../cstdint.h"
#include "../../../utility.h"
#include "../../platform.h"

namespace estd {

namespace internal {

// DEBT: This guy wants to play with estd::layer1::optional you can feel it
// We are doing this rather than a Null = T{} from the get go because some T won't
// play nice in that context
// TODO: layer1::string ought to only operate on value[0] null termination here, which is
// an ideal usage.  However it MAY be overzealously setting deeper chars to null.  Doublecheck
template <class T>
struct nullable_traits
{
    static constexpr bool is_null(const T& value)
    {
        return value == T{};
    }

    // DEBT: Don't want to use this, just helpful for init and erase
    static constexpr T get_null() { return T(); }

    ESTD_CPP_CONSTEXPR(17) static void set(T* value)
    {
        *value = T{};
    }
};


template <class Key, class T, class Hash, class KeyEqual, class Nullable>
class unordered_traits :
    // NOTE: Although EBO is interesting, spec suggests they aren't relevant here
    // i.e. https://en.cppreference.com/w/cpp/container/unordered_map/hash_function
    protected Hash,  // EBO
    protected KeyEqual  // EBO
{
public:
    using mapped_type = T;
    using key_type = Key;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using nullable = nullable_traits<Key>;

    //hasher& hash_function() { return *this; }
    //constexpr const hasher& hash_function() const { return *this; }
    //hasher hash_function() const { return {}; }

    static constexpr key_equal key_eq() { return {}; }
};


template <class Key, class T, class Hash, class KeyEqual, class Nullable>
struct unordered_map_traits : unordered_traits<Key, T, Hash, KeyEqual, Nullable>
{
    using base_type = unordered_traits<Key, T, Hash, KeyEqual, Nullable>;
    using traits = unordered_map_traits;
    using nullable = Nullable;
    using mapped_type = typename traits::mapped_type;
    using base_type::key_eq;

    // Mainly used for unordered_map since it has an unused area when key is null
    union meta
    {
        byte storage[sizeof(mapped_type)];

        struct
        {
            // aka "sparse" - exists specifically to mark as deleted, but physically unmoved
            uint16_t marked_for_gc : 1;
            // which bucket this empty slot *used to* belong to
            uint16_t bucket : 6;
        };

        //operator mapped_type& () { return * (mapped_type*) storage; }
        //constexpr operator const mapped_type& () const { return * (mapped_type*) storage; }

        mapped_type& mapped() { return * (mapped_type*) storage; }
    };

    using control_type = pair<typename traits::key_type, meta>;
    using value_type = pair<const typename traits::key_type, mapped_type>;

    /// @brief Checks for null OR sparse
    /// @param v
    /// @return
    template <class K, class T2>
    static constexpr bool is_null_or_sparse(const pair<K, T2>& v)
    {
        return nullable{}.is_null(v.first);
    }


    /// Determines if this ref is sparse - bucket must match also
    /// @param v
    /// @param n bucket#
    /// @return
    static constexpr bool is_sparse(const control_type& v, unsigned n)
    {
        return is_null_or_sparse(v) &&
            v.second.marked_for_gc &&
            v.second.bucket == n;
    }

    template <class K, class T2>
    static constexpr const Key& key(const pair<K, T2>& v) { return v.first; }

    static constexpr mapped_type& mapped(control_type& v)
    {
        return v.second.mapped();
    }

    static constexpr const mapped_type& mapped(const control_type& v)
    {
        return v.second.mapped();
    }

    /// Nulls out key
    /// @remark Does not run destructor
    static ESTD_CPP_CONSTEXPR(14) void set_null(control_type* v)
    {
        nullable{}.set(&v->first);
    }
};


template <class Key, class Hash, class KeyEqual, class Nullable>
struct unordered_set_traits : unordered_traits<Key, Key, Hash, KeyEqual, Nullable>
{
    using base_type = unordered_traits<Key, Key, Hash, KeyEqual, Nullable>;
    using typename base_type::nullable;

    using value_type = Key;
    using control_type = Key;
    using mapped_type = Key;

    static ESTD_CPP_CONSTEXPR(14) void set_null(value_type* v)
    {
        nullable{}.set(v);
    }

    static constexpr bool is_null_or_sparse(const value_type& v)
    {
        return nullable{}.is_null(v);
    }

    static constexpr bool is_sparse(const value_type&) { return false; }
    static constexpr bool is_sparse(const value_type&, unsigned) { return false; }

    static constexpr const Key& key(const control_type& v) { return v; }
    static constexpr mapped_type& mapped(control_type& v) { return v; }
    static constexpr const mapped_type& mapped(const control_type& v) { return v; }
};



}

}
