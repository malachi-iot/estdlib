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
    retry_item_base() = delete;
};

using retry_item = retry_item_base<int>;

template <class Key, class T = retry_item, size_t N = 10>
class retry_tracker
{
public:
    using key_type = Key;
    using map_type = layer1::unordered_map<Key, T, N>;
    // DEBT: Rename from pointer/const_pointer because that's really what T ought to map to
    using pointer = typename map_type::pointer;
    using const_pointer = typename map_type::const_pointer;
    using iter_type = typename map_type::iterator;
    using control_pointer = typename map_type::control_pointer;
    using find_result = typename map_type::template find_result<control_pointer>;

// Since this is unit testing, these are public
//private:
    /// General retry items tracked
    map_type tracked_;
    /// Who's next up for retry emit
    layer1::priority_queue<pointer, N> queue_;

    pointer gc_target_ {};

    // DEBT: Crude way to start decoupling from retry_item.  Still underway
    struct traits
    {
        using value_type = T;
        using timestamp = unsigned;

        static void calc_backoff(value_type& i)
        {
            // FIX: No way that '10' is a good value long term
            i.timestamp_ += 10;
        }

        static void ack_received(value_type& i, bool v)
        {
            i.ack_received_ = v;
        }

        constexpr static bool ack_received(const value_type& i)
        {
            return i.ack_received_;
        }

        constexpr static timestamp stamp(const value_type& i)
        {
            return i.timestamp_;
        }
    };

public:
    // Primarily for diagnostics, you wouldn't normally sniff around this guy.  Since
    // it's const, it's still a safe and sane call.
    const T* top() const
    {
        if(queue_.empty())  return nullptr;

        const_pointer it = queue_.top();
        return &it->second;
    }

    template <class ...Args>
    T* track(Key key, Args&&...args)
    {
        pair<iter_type, bool> r = tracked_.emplace(key, std::forward<Args>(args)...);

        if(r.second == false) return nullptr;

        queue_.emplace(estd::addressof(*r.first));
        return &r.first->second;
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

        if(timestamp < traits::stamp(value))    return 0;

        queue_.pop();

        if(ack_received)    *ack_received = traits::ack_received(value);

        // Don't requeue if ack was received
        if(traits::ack_received(value))
        {
            // ack_received is only set by 'incoming'.  Now we can fully
            // remove tracked as well
            // DEBT: Consider a consolidated 'erase_and_gc' which takes 'it' type.  Not doing
            // so yet because we need to document why we have separate mark/gc phases - especially
            // since the mark phase eagerly destructs
            tracked_.erase_ll(it);
            tracked_.gc_sparse_ll(it);

            return 1;
        }

        // we can still operate on value since it lives in tracker_
        // infinite retry
        traits::calc_backoff(value);

        // iterator also still valid, once again it lives in tracker_
        // DEBT: I think there's a "replace" mechanism in queue_
        queue_.emplace(it);

        return 1;
    }

    unsigned poll(typename traits::timestamp timestamp)
    {
        unsigned counter = 0;

        while(poll_one(timestamp) != 0) ++counter;

        return counter;
    }

    // DEBT: This would be better - at the moment our string hasher can't handle a char[]
    //template <class K>
    //void incoming(const K& k)
    T* incoming(const key_type& k)
    {
        // DEBT: Using find_ll because at the moment it's easier to determine
        // found/not found status
        find_result found = tracked_.find_ll(k);

        if(found.second != tracked_.npos())
        {
            traits::ack_received(found.first->second.mapped(), true);

            return &found.first->second.mapped();
        }

        return nullptr;
    }
};

}}
