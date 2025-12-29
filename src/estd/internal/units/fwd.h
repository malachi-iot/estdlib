#pragma once

#include "concepts.h"
#include "../macros.h"

// 28DEC25 - Noticing https://github.com/mpusz/mp-units really is a
// powerhouse.  Continuing with our own efforts since:
// 1.  Projection mode is uniquely useful (i.e. for J1939)
// 2.  Predictably able to feed into chrono
// 3.  Configurability of constructor really nice to have
// I haven't done research into mp-units, for all I know it could have
// all these things.

// Copying the playbook from chrono

namespace estd { namespace units { inline namespace v1 {

#if __cpp_concepts
inline namespace concepts {

using namespace estd::internal::units::concepts;

}
#endif

template <class T>
struct passthrough;

// Indicates to unit_base that narrowing might happen and to silently
// permit it
struct relaxed_narrow_t {};


namespace detail {

struct unit_base_tag {};

template <class Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F = passthrough<Rep>>
struct traits;

template <class Traits>
class unit;

}

template <typename Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F = passthrough<Rep>>
using unit = detail::unit<detail::traits<Rep, Period, Tag, F>>;

}}}

namespace estd { namespace internal { namespace units {

template <class Tag>
struct traits;

template <typename Int, Int sub>
struct subtractor;

template <typename Int, Int add>
struct adder;

using unit_base_tag = ::estd::units::v1::detail::unit_base_tag;

template <class T>
using passthrough = ::estd::units::v1::passthrough<T>;

// 1:1 ratio, passthrough always - used for readability, to reduce error verbosity
template <class Rep, class Tag>
struct basic_unit_traits;

namespace v2 {

template <class Traits>
using unit_base = ::estd::units::v1::detail::unit<Traits>;

}

inline namespace v1 {

template <typename Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F = passthrough<Rep>>
using unit_base = ::estd::units::unit<Rep, Period, Tag, F>;

}


template <class Rep, class Period>
struct cast_helper;

template <class, class>
struct compound_tag;

namespace si {

// DEBT: Do a concept here to filter by ratio
template <class Period, class Tag = void>
struct traits;

}

inline namespace literals {}


}}}

namespace estd { inline namespace literals { inline namespace units_literals {

}}}
