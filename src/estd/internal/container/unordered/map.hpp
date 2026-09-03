#pragma once

#include "base.hpp"
#include "map.h"

namespace estd { namespace internal {

template <class Container, class Traits>
void unordered_map<Container, Traits>::erase_ll(
    find_result<control_pointer> pos, bool auto_prune)
{
    const size_type n = pos.second;
    control_pointer control = pos.first;

    // side-effects Key to be empty
    destruct(control);

    // if block below either nulls or tombstones things, accordingly

    const const_control_pointer next = bump(control);

    // If no further bucket entries, do prune
    // that means:
    // - if next entry is an active item in a different bucket
    // - if next entry is a null entry
    // then we are clear to null out trailing sparse entries
    if(auto_prune && is_null_not_sparse(*next))
    {
        control_pointer start = container_.begin() + n;
        // nullify this and previous tombstones down to and including 'start'
        prune_sparse_ll(start, control);
    }
    else
    {
        using modes = unordered_map_control_enum::modes;

        // "mark and sweep" erase rather than erase (and swap) immediately in place.
        // More inline with spec, namely doesn't disrupt other iterators
        control->second.mode(modes::TOMBSTONE);

        // Look for last tombstone and mark it as eol
        find_and_mark_eol(control, n);
    }
}

template <class Container, class Traits>
bool unordered_map<Container, Traits>::find_and_mark_eol(control_pointer control, unsigned n)
{
    control_pointer tombstone = nullptr;

    for(control_pointer start = nullptr; control != start;
        control = bump(control))
    {
        // DEBT: Crude but effective way to avoid do/while for
        // begin & end being the same pointer
        if(start == nullptr)    start = container_.begin() + n;

        if(is_empty(*control))
        {
            const typename traits::meta& meta = control->second;
            using modes = unordered_map_control_enum::modes;
            const modes mode = meta.mode();

            // if null entry OR matching eol
            if(mode == modes::NULLED ||
                (mode == modes::EOL && meta.bucket() == n))
            {
                // Reaching here means foreign buckets reside in between here and previous
                // tombstone, meaning previous tombstone is the new eol

                // If no previous tombstone candidate found, then we already have the closest EOL/null
                // we can get
                if(tombstone == nullptr) return false;

                tombstone->second.bucket(n);
                tombstone->second.mode(modes::EOL);
                return true;
            }
            else if(tombstone == nullptr && mode == modes::TOMBSTONE)
            {
                // Reaching here means we've not yet got a tombstone candidate, and
                // current identified tombstone has no eol,
                // making it a viable candidate to be tagged with eol later
                tombstone = control;
            }
        }
        else
        {
            unsigned control_bucket = index(traits::key(*control));

            // If bucket extends this far, then the tombstone eol candidate we found is
            // no longer viable
            if(control_bucket == n) tombstone = nullptr;
        }
    }

    return false;
}

// NOT READY YET
template <class Container, class Traits>
bool unordered_map<Container, Traits>::find_and_mark_null(control_pointer control, unsigned n)
{
    // Presumes EOL calculation already occurred
    // TODO: Move forward from this position and investigate whether linear probing precludes
    // replacing an observed tombstone with null

    // Usually we'll expect the first 'control' to be EOL, but that's not a prerequisite
    // (but maybe it should be?)
    control_pointer candidate = nullptr;

    for(control_pointer start = nullptr; control != start;
        control = bump(control))
    {
        // DEBT: Crude but effective way to avoid do/while for
        // begin & end being the same pointer
        if(start == nullptr)    start = container_.begin() + n;

        if(is_empty(*control))
        {
            const typename traits::meta& meta = control->second;
            using modes = unordered_map_control_enum::modes;
            const modes mode = meta.mode();

            if(mode == modes::EOL && meta.bucket() == n)
            {
                assert(candidate == nullptr);
                candidate = control;
            }
        }
        else
        {
            unsigned control_bucket = index(traits::key(*control));

            // If no EOL found, assert is happy.  If EOL is found,
            // observed bucket must not be ours
            assert(candidate == nullptr || control_bucket != n);

            // TODO: Observe to see if encountered entries depend on linear probe supported
            // by our candidate - i.e., is our candidate anchored?
        }
    }

    return false;
}

template <class Container, class Traits>
void unordered_map<Container, Traits>::tombstone_to_null(control_pointer control)
{
    //const key_type& key = pos->first;
    //const size_type n = index(key);

    assert(is_empty(*control));

    using modes = unordered_map_control_enum::modes;

    control->second.mode(modes::NULLED);

    //prune_sparse_ll(control_pointer);
}


}}
