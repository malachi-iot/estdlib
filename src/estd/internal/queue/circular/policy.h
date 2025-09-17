#pragma once

#include "../../../array.h"
#include "../../../span.h"
#include "../../container/unordered/traits.h"

#include "enum.h"

namespace estd { namespace internal {

// 17SEP25 MB NOTE: I don't think we need nullable.  If it hasn't occurred why we
// might actually need this guy by 30SEP25, remove him.
template <class T, queue_options o, class Nullable = nullable_traits<T>>
struct circular_policy
{
    static constexpr queue_options type = o & queue_options::mask;
    static constexpr bool atomic = is_set(o & queue_options::atomic);
    constexpr static bool is_trivial = is_set(o & queue_options::trivial) || is_integral<T>::value
#if FEATURE_ESTD_IS_TRIVIAL
        || estd::is_trivial<T>::value
#endif
        ;

    using nullable = Nullable;
    static constexpr queue_options options = o;

    // Default iterator types - policy may override
    using iterator_type = T*;
    using const_iterator_type = const T*;
};

template <class T, size_t N, queue_options o>
struct array_circular_policy : circular_policy<T, o>
{
    using base_type = circular_policy<T, o>;
    using base_type::is_trivial;

    using uninitialized_array = internal::array<impl::uninitialized_array<T, N>>;

    // NOTE: Wanted to use raw array - for that, gymnastics are required to get at begin/end
    // with iterator sensibilities though.
    using container_type = conditional_t<is_trivial,
        estd::array<T, N>,
        uninitialized_array>;

    // DEBT: I don't think we need to switch this out.  What's the penalty for always
    // using T*?  Until I remember why I made this choice, keeping this commented out
    /*
    using iterator_type = conditional_t<is_trivial,
        T*, typename uninitialized_array::iterator>;

    using const_iterator_type = conditional_t<is_trivial,
        const T*, typename uninitialized_array::const_iterator>;    */
};

template <class T, size_t N, queue_options o>
struct span_circular_policy : circular_policy<T, o>
{
    using container_type = estd::span<T, N>;
};


}}