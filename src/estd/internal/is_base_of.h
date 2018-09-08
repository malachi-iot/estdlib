#pragma once

#include "../type_traits.h"
#include "../utility.h"

namespace estd {

namespace details {
    template <typename Base> estd::true_type is_base_of_test_func(const volatile Base*);
    template <typename Base> estd::false_type is_base_of_test_func(const volatile void*);
    template <typename Base, typename Derived>
    using pre_is_base_of = decltype(is_base_of_test_func<Base>(std::declval<Derived*>()));

    // with <experimental/type_traits>:
    // template <typename Base, typename Derived>
    // using pre_is_base_of2 = std::experimental::detected_or_t<std::true_type, pre_is_base_of, Base, Derived>;
    template <typename Base, typename Derived, typename = void>
    struct pre_is_base_of2 : public estd::true_type { };
    // note std::void_t is a C++17 feature
    template <typename Base, typename Derived>
    struct pre_is_base_of2<Base, Derived, estd::void_t<pre_is_base_of<Base, Derived>>> :
        public pre_is_base_of<Base, Derived> { };
}

template <typename Base, typename Derived>
struct is_base_of :
    public estd::conditional_t<
        estd::is_class<Base>::value && estd::is_class<Derived>::value,
        details::pre_is_base_of2<Base, Derived>,
        estd::false_type
    > { };

}
