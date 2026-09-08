#pragma once

#include "../../../optional.h"

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

        eol_helper helper;

        // Look for last tombstone and mark it as eol
        find_and_mark_eol(control, n, &helper);

        if(helper.null)    null_boomerang(helper, n);
    }
}

template <class Container, class Traits>
bool unordered_map<Container, Traits>::find_and_mark_eol(control_pointer control, unsigned n,
    eol_helper* helper)
{
    // EOL candidate
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

                if(helper)
                {
                    if(mode == modes::NULLED)   helper->null = tombstone;

                    helper->eol = tombstone;
                }

                // If no previous tombstone candidate found, then we already have the closest EOL/null
                // we can get
                if(tombstone == nullptr) return false;

                // Subsequent call to null_boomerang might then convert this into a NULL
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

template <class Container, class Traits>
void unordered_map<Container, Traits>::null_boomerang(const eol_helper& helper, unsigned n)
{
    // Key:bucket
    // T = Tombstone
    // N = Null
    // 1.  A:1, B:1, EOL:1, N
    // 2.  A:1, B:1, EOL:1, C:4, T, D:6, N
    // 3.  A:1, B:1, EOL:1, C:4, EOL:4, D:6, N
    // 4.  A:1, B:2, EOL:2, C:1, EOL:1, D:6, N
    // 5.  A:1, B:2, EOL:2, C:1, N

    control_pointer start = container_.begin() + n;
    [[maybe_unused]]
    control_pointer candidate = nullptr;
    control_pointer control = helper.null;

    // if null slot right at the beginning, we're already done
    if(control == start)    return;

    using modes = unordered_map_control_enum::modes;

    // where one more more displaced values are bunched together OR
    // there's a tombstone in the middle of a bucket (vs the end).
    // needs a better name
    bool intermingled = false;
    estd::layer1::optional<modes, static_cast<modes>(-1)> hopeful_mode;
    // bucket (hashed key) of last active entry
    estd::layer1::optional<unsigned, 0xFFFF> last_bucket;

    // In fact multiple buckets can be active at once with enough intermingling.  We are not
    // advanced enough for that case just yet.  For the time being, active_bucket is the lowest
    // encountered bucket# whose domain we are still inside
    estd::layer1::optional<unsigned, 0xFFFF> active_bucket;

    auto assign_candidate = [&](unsigned n)
    {
        if(hopeful_mode.has_value() == false) return;

        candidate->second.mode(*hopeful_mode);
        // DEBT: Only assign this if it's EOL
        candidate->second.bucket(n);

        hopeful_mode.reset();
        candidate = nullptr;
    };

    // indicates the next empty slot is null vs a tombstone
    // since we expect a null entry from the start, we set this to true
    //bool next_empty_is_null = true;

    // Tombstones encountered in this direction might be convertible to EOL/null
    // Walk down to and including starting bucket entry
    do  // NOLINT
    {
        control_pointer trailing = control;
        control = rbump(control);

        if(is_empty(*control))
        {
            typename traits::meta& meta = control->second;
            const modes mode = meta.mode();

            // null_boomerang expects that there are NO nulls between initial 'control'
            // and bucket start
            assert(mode != modes::NULLED);

            // Previous candidate was selected, let's see what we can do about him
            if(candidate)
            {
                if(!intermingled)   assign_candidate(n);
            }

            // Any EOL next to a NULLED is automatically converted to NULLED also
            // (Scenario 1)
            if(mode == modes::EOL && is_null_not_sparse(*trailing))
            {
                meta.mode(modes::NULLED);
                hopeful_mode = modes::NULLED;
                candidate = nullptr;
            }
            else
            {
                // FIX: We can't be sure null will be valid here without extra checking
                hopeful_mode = modes::NULLED;
                candidate = control;
            }
        }
        else
        {
            unsigned natural_bucket = control - container_.begin();
            unsigned control_bucket = index(traits::key(*control));

            // Active bucket changes if:
            // 1.  We had none yet
            // 2.  Encountered bucket index is smaller than current active one
            if(active_bucket.has_value() == false)
            {
                active_bucket = control_bucket;
            }
            else if(natural_bucket < *active_bucket)
            {
                // Moving to new bucket boundary.  Not innately intermingled due to
                // https://malachi.atlassian.net/wiki/x/AYD0DQ section 3.3.3.
                intermingled = false;

                // Fully leaving one bucket region for another means it's time to try to
                // write our null/eol candidate
                if(candidate)   assign_candidate(*active_bucket);

                // Observe that we don't assign to active_bucket.  active_bucket
                // mainly helps us determine what to do next, but we still are interested
                // in actual active item hash
                active_bucket = control_bucket;

                // We could deduce some level of intermingling by seeing if control_bucket ==
                // natural bucket.  However, that could yield a hard false but not much conclusive
                // if it's true (might still be false), so starting with false and letting
                // further investigation prove us wrong
            }
            else if(control_bucket > *active_bucket)
            {
                // intermingled buckets, we only support one, so keep the lowest#
                // since we only track ONE active_bucket, we can't easily determine null, so
                // we are stuck with EOL mode of just one of the many simultanoues buckets
                intermingled = true;
                // we already have lowest bucket#
                //active_bucket = control_bucket;
                hopeful_mode = modes::EOL;
            }

            if(candidate && control == start)
                // DEBT: Pick up bucket from control_bucket, filtering out by intermingled somehow
                assign_candidate(n);

            /*
            if(control_bucket != last_bucket)
            {
                // if bucket changeover occurs after we've left natural bucket spot, then that
                // means a tagged tombstone can be an EOL
            }   */

            last_bucket = static_cast<int>(control_bucket);

            /*
            if(control_bucket < natural_bucket)
            {
                // Reaching here means aggressive linear probing occurred and placed items outside of their
                // natural location.  That means
            }

            // If bucket extends this far, then the tombstone eol candidate we found is
            // no longer viable
            if(control_bucket == n) candidate = nullptr;    */
        }

    }
    while(control != start);
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
