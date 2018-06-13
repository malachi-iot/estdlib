#pragma once

namespace estd { namespace internal {

// base class for dynamic_array
// performs a lot of dynamic_array duties, but behaves as a read-only entity
// though constness is not specifically enforced, as writable entities will
// derive from this
template <class THelper>
class runtime_array
{
public:
};

}}
