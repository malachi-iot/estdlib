#pragma once

namespace estd {

struct in_place_t
{

};

template <class T>
struct in_place_type_t
{

};

// NOTE: No in_place_index_t because that may need a cross-include for std::size

}