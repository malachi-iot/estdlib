#pragma once

namespace estd {

namespace internal {

struct in_place_tag {};

}

struct in_place_t : internal::in_place_tag
{

};

template <class T>
struct in_place_type_t : internal::in_place_tag
{

};

// DEBT: in_place_index_t wants std::size_t, but that may need a cross-include
template <unsigned>
struct in_place_index_t : internal::in_place_tag
{};

}
