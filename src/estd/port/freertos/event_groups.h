#pragma once

#include "wrapper/event_groups.h"

#include "fwd.h"
#include "chrono.h"

namespace estd { namespace freertos {

namespace internal {

struct event_group : wrapper::event_group
{
    typedef wrapper::event_group base_type;
    typedef estd::chrono::freertos_clock::duration duration;
    
    event_group(EventGroupHandle_t h) : base_type(h) {}
    event_group(wrapper::event_group e) : base_type(e) {}

    EventBits_t wait_bits(
                       const EventBits_t uxBitsToWaitFor,
                       const bool xClearOnExit,
                       const bool xWaitForAllBits,
                       duration wait)
    {
        return base_type::wait_bits(uxBitsToWaitFor, xClearOnExit, xWaitForAllBits, wait.count());
    }

    EventBits_t sync(const EventBits_t uxBitsToSet,
                              const EventBits_t uxBitsToWaitFor,
                              duration wait)
    {
        return base_type::sync(uxBitsToSet, uxBitsToWaitFor, wait.count());
    }
};

}

#if configSUPPORT_DYNAMIC_ALLOCATION
template <>
class event_group<false> : public internal::event_group
{
    typedef internal::event_group base_type;

protected:
    event_group(EventGroupHandle_t h) : base_type(h) {}

public:
    event_group() : base_type(base_type::create()) {}
    ~event_group() { base_type::free(); }

};
#endif

#if configSUPPORT_STATIC_ALLOCATION
template <>
class event_group<true> : public event_group<false>
{
    typedef event_group<false> base_type;

    StaticEventGroup_t storage;

public:
    event_group() : base_type(base_type::create(&storage)) {}
};
#endif

}}