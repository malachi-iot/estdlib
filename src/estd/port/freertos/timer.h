// NOTE: Unlike some other header files in this folder, this does not have a 1:1 mapping with std
#pragma once

#include "fwd.h"
#include "wrapper/timer.h"

namespace estd { namespace freertos { 

template <>
class timer<false> : public internal::timer
{
    typedef internal::timer base_type;

public:
    timer(const char* const name, duration period, bool auto_reload, void* const id,
        TimerCallbackFunction_t callback) :
        base_type(wrapper::timer::create(name, period.count(), auto_reload, id, callback))
    {

    }

    ~timer()
    {
        // DEBT: Feed this via a policy
        free(estd::chrono::milliseconds(100));
    }
};


// FIX: Not well tested, because unit test crashes when trying it.  Probably
// unit test frees its stack-allocated StaticTimer_t before timer is truly done with it
template <>
class timer<true> : public internal::timer
{
    typedef internal::timer base_type;

    StaticTimer_t storage;

public:
    timer(const char* const name, duration period, bool auto_reload, void* const id,
        TimerCallbackFunction_t callback) :
        base_type(wrapper::timer::create(name, period.count(), auto_reload, id, callback, &storage))
    {

    }

    ~timer()
    {
        // DEBT: Feed this via a policy
        free(estd::chrono::milliseconds(100));
    }
};

}}