#pragma once

namespace estd {

struct in_place_t
{

};

template <class T>
struct in_place_type_t
{

};

// DEBT: in_place_index_t wants std::size_t, but that may need a cross-include
template <unsigned>
struct in_place_index_t {};

}
