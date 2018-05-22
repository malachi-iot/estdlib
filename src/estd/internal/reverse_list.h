#pragma once

#include "list_node.h"

namespace estd { namespace internal {

// need TBase trickery so that we can share stateful traits without virtualizing the base
// classes
template<class TBase>
class reverse_list : public TBase
{
protected:
    typedef TBase base_t;

public:
    typedef typename base_t::allocator_t allocator_t;
    typedef typename base_t::node_handle node_handle;
    typedef typename base_t::iterator iterator;
    typedef typename base_t::const_iterator const_iterator;
    typedef typename base_t::node_type node_type;
    typedef typename base_t::nv_reference nv_reference;

protected:
    node_handle m_back;

    void set_back(node_handle new_back)
    {
        set_prev(new_back, m_back);

        m_back = new_back;
    }

    void set_prev(node_handle _node, node_handle prev)
    {
        node_type& node = base_t::alloc_lock(_node);

        base_t::traits.prev(node, prev);

        base_t::alloc_unlock(_node);
    }

public:
    reverse_list(allocator_t* allocator = NULLPTR) :
        base_t(allocator),
        m_back(base_t::after_end_node())
    {}

    nv_reference back()
    {
        node_type& back_value = base_t::alloc_lock(m_back);
        nv_reference retval = base_t::traits.value(back_value);

        base_t::alloc_unlock(m_back);

        return retval;
    }
};

}}
