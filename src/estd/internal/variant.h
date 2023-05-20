#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "utility.h"
#include "raw/variant.h"
#include "../tuple.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {


// DEBT: Move type_at_index and index_of_type elsewhere

// Very similar to std::variant_alternative
template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;

template <class T, int I, class ...TArgs>
struct index_of_type_helper;

template <class T, int I>
struct index_of_type_helper<T, I>
{
    static constexpr bool match = false;
    static constexpr int index = -1;
};

template <class T, int I, class T2, class ...TArgs>
struct index_of_type_helper<T, I, T2, TArgs...>
{
    typedef index_of_type_helper<T, I + 1, TArgs...> up_one;

    static constexpr bool match = is_same<T, T2>::value;
    static constexpr int index = match ? I : up_one::index;
    static constexpr bool multiple = match & up_one::index != -1;
};

template <class T, class ...TArgs>
using index_of_type = index_of_type_helper<T, 0, TArgs...>;

template <bool trivial, class ...T>
struct variant_storage;

template <class ...Types>
class variant;


template <int index, class ...TArgs>
constexpr type_at_index<index, TArgs...>* get_if(variant<TArgs...>& vs)
{
    return vs.index() != (unsigned)index ? nullptr : vs.template get<index>();
}

// DEBT: true std code throws exception on index mismatch here - we need to reflect error
// state somehow
template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
{
    typedef type_at_index<index, TArgs...>* pointer;
    return * (pointer)vs.storage.raw;
}

template <int index, bool trivial, class ...TArgs>
const type_at_index<index, TArgs...>& get(const variant_storage<trivial, TArgs...>& vs)
{
    typedef const type_at_index<index, TArgs...>* const_pointer;
    return * (const_pointer)vs.storage.raw;
}

template <class T, bool trivial, class ...Types>
T& get(variant_storage<trivial, Types...>& vs)
{
    return * vs.template get<T>();
}

template <class T, bool trivial, class ...Types>
constexpr const T& get(const variant_storage<trivial, Types...>& vs)
{
    return * vs.template get<T>();
}

template <class T, class ...Types>
typename add_pointer<T>::type get_if(variant<Types...>& vs)
{
    int i = index_of_type<T, Types...>::index;

    if(i == -1 || vs.index() != (unsigned)i) return nullptr;

    return vs.template get<T>();
}





// DEBT: Currently variant_storage is clunky and fine-tuned for consumption by
// 'expected'

// Very similar to c++20 flavor, but importantly returns monostate instead
template <class T2, class ...TArgs>
inline static monostate construct_at(void* placement, TArgs&&...args)
{
    new (placement) T2(std::forward<TArgs>(args)...);
    return {};
}


template <bool trivial, class ...TArgs>
union variant_union;

template <class ...T>
union variant_union<false, T...>
{
    estd::byte raw[sizeof(typename largest_type<T...>::type)];
};

// Not 100% needed, but I like that I can see values more
// easily in the debugger this way
template <class T1, class T2>
union variant_union<true, T1, T2>
{
    T1 t1;
    T2 t2;
    byte raw[0];
};

template <class T1, class T2, class T3>
union variant_union<true, T1, T2, T3>
{
    T1 t1;
    T2 t2;
    T3 t3;
    byte raw[0];
};


template <bool trivial, class ...Types>
struct variant_storage
{
    typedef tuple<Types...> tuple_type;
    static constexpr bool is_trivial = trivial;

    union
    {
        variant_union<trivial, Types...> storage;
        monostate dummy;
    };

    variant_storage() = default;

    template <unsigned index, class ...TArgs>
    constexpr variant_storage(estd::in_place_index_t<index>, TArgs&&...args) :
        dummy{
            construct_at<type_at_index<index, Types...>>
                (storage.raw, std::forward<TArgs>(args)...)}
    {
    }

    template <unsigned index>
    type_at_index<index, Types...>* get()
    {
        return (type_at_index<index, Types...>*) storage.raw;
    }

    template <class T>
    T* get() { return (T*) storage.raw; }

    template <class T>
    constexpr T* get() const { return (T*) storage.raw; }
};

template <class ...T>
using variant_storage2 = variant_storage<are_trivial<T...>::value, T...>;


template <class ...Types>
class variant : public variant_storage2<Types...>
{
    using base_type = variant_storage2<Types...>;
    using size_type = std::size_t;

    template <class T2>
    using index_of_type = estd::internal::index_of_type<T2, Types...>;

    size_type index_;

    // DEBT: Not a great name
    template <class T>
    T* set_index()
    {
        index_ = index_of_type<T>::index;
        return base_type::template get<T>();
    }

public:
    constexpr variant() :
        base_type(in_place_index_t<0>{}),
        index_{0}
    {}

    template <unsigned index, class ...TArgs>
    constexpr explicit variant(in_place_index_t<index>, TArgs&&...args) :
        base_type(in_place_index_t<index>{}, std::forward<TArgs>(args)...),
        index_{index}
    {}

    template <class T2, class ...TArgs>
    constexpr explicit variant(in_place_type_t<T2>, TArgs&&...args) :
        base_type(
            in_place_index_t<index_of_type<T2>::index>{},
            std::forward<TArgs>(args)...),
        index_{index_of_type<T2>::index}
    {}

    variant(variant&& move_from) noexcept:
        index_{move_from.index()}
    {
        //constexpr unsigned i = move_from.index();
        //auto& v = get<i>(*this);
        // TODO: Need to std::move on the particular idx/type
    }


    template <class T>
    variant& operator=(T&& v)
    {
        *set_index<T>() = v;
        return *this;
    }

    constexpr size_type index() const { return index_; }

    template <class T, class ...TArgs>
    T& emplace(TArgs&&...args)
    {
        T* v = set_index<T>();

        new (v) T(std::forward<TArgs>(args)...);

        return *v;
    }
};


template< class T, class... Types >
constexpr bool holds_alternative(const variant<Types...>& v) noexcept
{
    return v.index() == (unsigned) index_of_type<T, Types...>::index;
}

}
#endif

}
