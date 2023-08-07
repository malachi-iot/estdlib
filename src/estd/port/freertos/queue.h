#pragma once

#include "../../array.h"

#include "chrono.h"
#include "fwd.h"
#include "wrapper/queue.h"

namespace estd { namespace freertos {

namespace detail {

template <class T>
struct queue
{
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef estd::chrono::freertos_clock::duration duration;

protected:
    wrapper::queue q;

    queue(QueueHandle_t q) : q(q) {}
    ~queue()
    {
        q.free();
    }

public:
    UBaseType_t messages_waiting() const
    {
        return q.messages_waiting();
    }

    bool send(const_reference v, duration timeout)
    {
        return q.send(&v, timeout.count()) == pdTRUE;
    }

    bool send(const_reference v)
    {
        return q.send(&v, portMAX_DELAY) == pdTRUE;
    }

    bool receive(pointer v, duration timeout)
    {
        return q.receive(v, timeout.count()) == pdTRUE;
    }

    bool receive(pointer v)
    {
        return q.receive(v, portMAX_DELAY) == pdTRUE;
    }

    BaseType_t send_from_isr(const_reference v, BaseType_t* pxHigherPriorityTaskWoken = nullptr)
    {
        return q.send_from_isr(&v, pxHigherPriorityTaskWoken);
    }
};

}


template <class T>
class queue<T, false> : public detail::queue<T>
{
    typedef detail::queue<T> base_type;
    typedef typename base_type::value_type value_type;

public:
    queue(unsigned queue_length) :
        base_type(
            wrapper::queue::create(queue_length, sizeof(value_type)))
    {

    }
};

#if configSUPPORT_STATIC_ALLOCATION
template <class T>
class queue<T, true> : public detail::queue<T>
{
    StaticQueue_t static_queue;

    typedef detail::queue<T> base_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::pointer pointer;

public:
    queue(pointer storage, unsigned queue_length) : 
        base_type(wrapper::queue::create(
            queue_length,
            sizeof(value_type),
            reinterpret_cast<uint8_t*>(storage),
            &static_queue))
    {

    }

    // UNTESTED
    BaseType_t static_buffers(
        uint8_t** ppucQueueStorage,
        StaticQueue_t** ppxStaticQueue) const
    {
        return xQueueGetStaticBuffers(base_type::q, ppucQueueStorage, ppxStaticQueue);
    }
};
#endif

namespace layer1 {

template <class T, unsigned queue_length>
class queue : public freertos::queue<T, true>
{
    typedef freertos::queue<T, true> base_type;

    estd::array<T, queue_length> storage;

public:
    queue() : base_type(storage.data(), queue_length) {}
};

}


}}