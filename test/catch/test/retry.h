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
    using const_pointer = typename map_type::const_pointer;
    using iter_type = typename map_type::iterator;

// Since this is unit testing, these are public
//private:
    /// General retry items tracked
    map_type tracked_;
    /// Who's next up for retry emit
    layer1::priority_queue<pointer, N> queue_;

    pointer gc_target_ {};

public:
    // Primarily for diagnostics, you wouldn't normally sniff around this guy.  Since
    // it's const, it's still a safe and sane call.
    const T* top() const
    {
        if(queue_.empty())  return nullptr;

        const_pointer it = queue_.top();
        return &it->second;
    }

    bool track(Key key, const T& value)
    {
        pair<iter_type, bool> r = tracked_.emplace(key, value);

        if(r.second == false) return false;

        queue_.emplace(estd::addressof(*r.first));
        return true;
    }

    // TBD
    void gc_one()
    {
        tracked_.gc_active_ll(gc_target_);
    }

    // boost-style
    ///
    /// @param timestamp
    /// @param ack_received debug-oriented, indicate whether processed item was ACK'd and therefore removed
    /// @return 0 or 1 item process counter
    unsigned poll_one(int timestamp, bool* ack_received = nullptr)
    {
        if(queue_.empty())  return 0;

        pointer it = queue_.top();
        T& value = it->second;

        // DEBT: Hard wired to test::retry_item

        if(timestamp < value.timestamp_)    return 0;

        queue_.pop();

        if(ack_received)    *ack_received = value.ack_received_;

        // Don't requeue if ack was received
        if(value.ack_received_)
        {
            // ack_received is only set by 'incoming', who also marks this
            // guy for deletion
            tracked_.gc_sparse_ll(it);

            return 1;
        }

        // we can still operate on value since it lives in tracker_
        // infinite retry
        // FIX: No way that '10' is a good value long term
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
