#pragma once

#include <estd/internal/priority_queue.h>
#include <estd/internal/container/unordered/fwd.h>

#include "nontrivial.h"

namespace estd { namespace test {

template <typename TimeStamp, class Impl = monostate>
struct retry_item_base : Impl
{
    TimeStamp timestamp_;
    bool ack_received_{false};

    explicit constexpr retry_item_base(TimeStamp timestamp) : timestamp_{timestamp}  {}
};

using retry_item = retry_item_base<int>;

template <class Key, class T = retry_item, size_t N = 10>
class retry_tracker
{
public:
    using key_type = Key;
    using map_type = layer1::unordered_map<Key, T, N>;
    using pointer = typename map_type::pointer;
    using iter_type = typename map_type::iterator;

// Since this is unit testing, these are public
//private:
    map_type tracked_;
    layer1::priority_queue<pointer, N> queue_;

    pointer gc_target_ {};

public:
    void track(Key key, const T& value)
    {
        pair<iter_type, bool> r = tracked_.emplace(key, value);

        if(r.second == false) return;

        queue_.emplace(estd::addressof(*r.first));
    }

    // TBD
    void gc_one()
    {
        tracked_.gc_active_ll(gc_target_);
    }

    // boost-style
    unsigned poll_one(int timestamp)
    {
        if(queue_.empty())  return 0;

        pointer it = queue_.top();
        T& value = it->second;

        // DEBT: Hard wired to test::retry_item

        if(timestamp < value.timestamp_)    return 0;

        queue_.pop();

        // Don't requeue when ack has been received
        if(value.ack_received_)
        {
            // ack_received is only set by 'incoming', who also marks this
            // guy for deletion
            tracked_.gc_sparse_ll(it);

            return 1;
        }

        // we can still operate on value since it lives in tracker_
        // infinite retry
        value.timestamp_ += 10;

        // iterator also still valid, once again it lives in tracker_
        // DEBT: I think there's a "replace" mechanism in queue_
        queue_.emplace(it);

        return 1;
    }

    unsigned poll(int timestamp)
    {
        unsigned counter = 0;

        while(poll_one(timestamp) != 0) ++counter;

        return counter;
    }

    // DEBT: This would be better - at the moment our string hasher can't handle a char[]
    //template <class K>
    //void incoming(const K& k)
    void incoming(const key_type& k)
    {
        /*
        value_type found = tracked_.find(k);

        if(found != tracked_.cend())
        {
            found->second.ack_received_ = true;
        }   */

        // DEBT: Using find_ll because at the moment it's easier to determine
        // found/not found status
        auto found = tracked_.find_ll(k);

        if(found.second != tracked_.npos())
        {
            found.first->second.mapped().ack_received_ = true;

            // mark this guy for deletion (but don't delete yet)
            tracked_.erase_ll(found);
        }
    }
};

}}
