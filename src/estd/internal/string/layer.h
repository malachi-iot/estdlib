#pragma once

#include "../../string_view.h"

// Things that only pertain to layer1-layer3 strings

namespace estd { namespace internal {

namespace mixins {

template <class Derived, class Traits>
struct string
{
    // Using non-standard char_type on purpose to avoid confusing concepts::Container
    // (amibiguity)
    using char_type = typename Traits::char_type;
    //using view_type = basic_string_view<char_type, Traits>;

    template <class Impl>
    ESTD_CPP_CONSTEXPR(14) Derived& operator=(const estd::detail::basic_string<Impl>& copy_from)
    {
        auto self = static_cast<const Derived*>(this);
        self->operator =(copy_from);
        return *this;
    }

    ESTD_ALLOC_INLINE
    ESTD_CPP_CONSTEXPR(14) operator basic_string_view<char_type, Traits>() const
    {
        auto self = static_cast<const Derived*>(this);

        return { self->data(), self->size() };
    }

#if FEATURE_STD_STRING_VIEW
    ESTD_ALLOC_INLINE
    ESTD_CPP_CONSTEXPR(14) operator std::basic_string_view<char_type, Traits>() const
    {
        auto self = static_cast<const Derived*>(this);

        return { self->data(), size_t(self->size()) };
    }
#endif
};

}

}}
