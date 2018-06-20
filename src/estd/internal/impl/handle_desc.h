#pragma once

#include "../../traits/allocator_traits.h"

namespace estd { namespace internal { namespace impl {

template <class TValue, bool is_present, TValue default_value>
class value_evaporator;

template <class TValue, TValue default_value>
class value_evaporator<TValue, true, default_value>
{

};

template <class TValue, TValue default_value>
class value_evaporator<TValue, false, default_value>
{
protected:
};

// https://en.cppreference.com/w/cpp/language/ebo we can have specialized base classes which are empty
// and don't hurt our sizing
template <class TAllocator, bool is_stateful>
class allocator_descriptor_base;


// TAllocator could be a ref here
template <class TAllocator>
class allocator_descriptor_base<TAllocator, true>
{
    TAllocator allocator;

protected:
    // NOTE: variadic would be nice, but obviously not always available
    template <class TAllocatorParameter>
    allocator_descriptor_base(TAllocatorParameter& p) :
        allocator(p) {}

    // Not unusual for a stateful allocator to default construct itself just
    // how we want it
    allocator_descriptor_base() {}

public:
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;
    typedef typename allocator_type::handle_type handle_type;

    // Would be nice if we could const this, but for stateful allocators that's not reasonable
    allocator_type& get_allocator() { return allocator; }

    const allocator_type& get_allocator() const { return allocator; }

protected:
};


template <class TAllocator>
struct allocator_descriptor_base<TAllocator, false>
{
    typedef TAllocator allocator_type;

    // NOTE: odd, but OK.  Since we're stateless, we can return what otherwise
    // would be an invalid reference
    allocator_type& get_allocator() const { return TAllocator(); }
};

// singular technically doesn't track a handle
// TODO: refactor to utilize value_evaporator (get_allocator, too)
template <class TAllocator, bool is_singular>
class handle_descriptor_base;

template <class TAllocator>
class handle_descriptor_base<TAllocator, true>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;

protected:
    handle_descriptor_base(bool) {}

    typedef typename allocator_type::handle_type handle_type;

    void handle(bool) {}

    value_type& lock(allocator_type& a, size_type pos = 0, size_type len = 0)
    {
        return a.lock(true, pos, len);
    }

    void unlock(allocator_type& a) { a.unlock(true); }

public:
    bool handle() const { return true; }
};


template <class TAllocator>
struct handle_descriptor_base<TAllocator, false>
{
    typedef typename remove_reference<TAllocator>::type allocator_type;
    typedef typename allocator_type::handle_type handle_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;

private:
    handle_type m_handle;

protected:
    void handle(const handle_type& h) { m_handle = h; }

    handle_descriptor_base(const handle_type& h) : m_handle(h) {}

    value_type& lock(allocator_type& a, size_type pos = 0, size_type len = 0)
    {
        return a.lock(m_handle, pos, len);
    }

    void unlock(allocator_type& a) { a.unlock(m_handle); }

public:
    bool handle() const { return m_handle; }
};


}}}
