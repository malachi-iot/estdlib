#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "raw/utility.h"
#include "variadic.h"
#include "raw/variant.h"
#include "../tuple.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {


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
    return * vs.template get<index>();
}

template <int index, bool trivial, class ...TArgs>
const type_at_index<index, TArgs...>& get(const variant_storage<trivial, TArgs...>& vs)
{
    return * vs.template get<index>();
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





// Very similar to c++20 flavor, but importantly returns monostate instead
template <class T2, class ...TArgs>
inline static monostate construct_at(void* placement, TArgs&&...args)
{
    new (placement) T2(std::forward<TArgs>(args)...);
    return {};
}

// DEBT: Supposed to be an inline variable, but we want c++11 compat
constexpr unsigned variant_npos()
{
    return (unsigned)-1;
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

    template <class T>
    using ensure_type_t = typename ensure_type<T, Types...>::type;

private:
    union
    {
        variant_union<trivial, Types...> storage;
        monostate dummy;
    };

public:
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

    template <unsigned index>
    constexpr type_at_index<index, Types...>* get() const
    {
        return (type_at_index<index, Types...>*) storage.raw;
    }

    template <class T>
    ensure_type_t<T>* get() { return (T*) storage.raw; }

    template <class T>
    constexpr ensure_type_t<T>* get() const { return (T*) storage.raw; }

    template <int I, typename F, typename enabled = estd::enable_if_t<I == 0, bool> >
    static constexpr bool visit_old(F&&, int) { return{}; }

    template <int I = sizeof...(Types), typename F,
             typename enabled = estd::enable_if_t<(I > 0), bool> >
    void visit_old(F&& f, const int index, bool = true)
    {
        if(I - 1 == index)
            f(get<I - 1>());
        else
            visit_old<I - 1>(std::forward<F>(f), index);
    }

    template <int I, typename F>
    static constexpr bool visit(F&&, int) { return{}; }

    template <int I, typename F, class T, class... TArgs>
    void visit(F&& f, int index)
    {
        if(I == index)
            f(get<T>());
        else
            visit<I + 1, F, TArgs...>(std::forward<F>(f), index);
    }

    template <typename F>
    void visit(F&& f, int index)
    {
        visit<0, F, Types...>(std::forward<F>(f), index);
    }
};

template <class ...T>
using variant_storage2 = variant_storage<are_trivial<T...>::value, T...>;

// NOTE: Using regular functions for F&& style functors doesn't
// seem to work, perhaps in particular due to class T parameter?
struct destroyer_functor
{
    template <class T>
    void operator()(T* t) const
    {
        t->~T();
    }
};

template <class ...Types>
class variant : public variant_storage2<Types...>
{
    using base_type = variant_storage2<Types...>;
    using size_type = std::size_t;

    struct mover_functor
    {
        variant* const parent;

        template <class T>
        void operator()(T* move_from)
        {
            T* move_to = parent->template get<T>();
            new (move_to) T(std::move(*move_from));
        }
    };

    template <class T2>
    using index_of_type = estd::internal::index_of_type<T2, Types...>;

    template <int I>
    using type_at_index = estd::internal::type_at_index<I, Types...>;

    size_type index_;

    // DEBT: Not a great name
    template <class T>
    T* set_index()
    {
        index_ = index_of_type<T>::index;
        return base_type::template get<T>();
    }

    /*
    template <class T>
    static void destroy(T* t)
    {
        t->~T();
    }   */

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

    constexpr bool valueless_by_exception() const
    {
        return index_ == variant_npos();
    }

    template <class F>
    void visit(F&& f)
    {
        base_type::visit(std::move(f), index_);
    }

    ~variant()
    {
        //auto v = base_type::template get<index_>();
        //apply<sizeof...(Types) - 1>(destroyer_functor{});
        //apply<sizeof...(Types) - 1>(&destroy);
        if(!valueless_by_exception())
            visit(destroyer_functor{});
    }

    variant(variant&& move_from) noexcept:
        index_{move_from.index()}
    {
        mover_functor f{this};
        move_from.visit(f);

        // DEBT: Setting to 'valueless_by_exception'.  Docs don't indicate to do this,
        // but if we don't then the move_from variant destructor will try to run the dtor
        // of the alternative again - which I suppose is safe, but feels wrong somehow
        move_from.index_ = variant_npos();
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
