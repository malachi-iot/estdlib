#include "fwd.h"

// In response to https://github.com/malachi-iot/estdlib/issues/11

namespace estd { namespace internal { namespace list {

// If we want an allocator (non intrusive), it will come along as part of traits
// though we might need an Impl for stateful allocators
template <class Node, class Data>
struct intrusive_traits
{
    // Get next ptr
    static Node* next(const Node* node)
    {
        return node->next();
    }

    // Re-assign next ptr
    static void next(Node* node, Node* v)
    {
        node->next(v);
    }

    // Experimental - seeing if we want to somehow grab data from
    // elsewhere.  Probably not.
    static Data& data(Node* node)
    {
        return *node;
    }

    static const Data& data(const Node* node)
    {
        return *node;
    }
};

}}}