#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "fwd/variant.h"
#include "raw/utility.h"
#include "raw/variant.h"
#include "feature/variant.h"

#include "variadic.h"

#include <cstdlib>
#include <new>

#if __cpp_exceptions
#include <exception>
#endif

#if __cpp_variadic_templates
namespace estd {

#if __cpp_exceptions
class bad_variant_access : std::exception
{
public:
};
#endif

template <class... Types>
struct variant_size<internal::variant_storage<Types...> > :
    internal::variadic_size<Types...> {};

template <unsigned I, class... Types>
struct variant_alternative<I, internal::variant_storage<Types...> > :
    type_identity<internal::type_at_index<I, Types...>> { };

namespace internal {

template <size_t index, class ...Types>
constexpr bool holds_index(const variant<Types...>* vs)
{
    return vs != nullptr && vs->index() == index;
}


template <class T, class ...Types>
constexpr bool holds_type(const variant<Types...>* vs)
{
    typedef typename select_type<T, Types...>::selected_indices selected;

    // NOTE: size() check is redundant, because compile time check for 'first()' below fails
    // if no items are present.  This does not clash with spec, which indicates
    // this is "ill-formed if T is not a unique element"
    return selected::size() != 0 &&
           (vs != nullptr && vs->index() == selected::first());
}




// DEBT: true std code throws exception on index mismatch here - we need to reflect error
// state somehow
template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...>& get(variant_storage_base<trivial, TArgs...>& vs)
{
    return * vs.template get<index>();
}

template <int index, bool trivial, class ...TArgs>
const type_at_index<index, TArgs...>& get(const variant_storage_base<trivial, TArgs...>& vs)
{
    return * vs.template get<index>();
}

template <class T, bool trivial, class ...Types>
T& get(variant_storage_base<trivial, Types...>& vs)
{
    return * vs.template get<T>();
}

template <class T, bool trivial, class ...Types>
constexpr const T& get(const variant_storage_base<trivial, Types...>& vs)
{
    return * vs.template get<T>();
}

template <int index, class ...Types>
void assert_index_matches(const variant<Types...>& v)
{
#if __cpp_exceptions
    if(v.index() != index) throw bad_variant_access();
#else
    // NOTE: Not tested yet
    if(v.index() != index) std::abort();
#endif
}


// DEBT: Supposed to be an inline variable, but we want c++11 compat
constexpr unsigned variant_npos()
{
    return (unsigned)-1;
}



// Very similar to c++20 flavor, but importantly returns monostate instead
template <class T2, class ...TArgs>
inline static monostate construct_at(void* placement, TArgs&&...args)
{
    new (placement) T2(std::forward<TArgs>(args)...);
    return {};
}

template <class ...T>
union variant_union<false, T...>
{
    estd::byte raw[sizeof(typename largest_type<T...>::type)];
};

template <>
union variant_union<true>
{

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

template <class T1, class T2, class T3, class T4>
union variant_union<true, T1, T2, T3, T4>
{
    T1 t1;
    T2 t2;
    T3 t3;
    T4 t4;
    byte raw[0];
};


struct variant_storage_getter_functor
{
    template <unsigned I, class T, bool trivial, class ...TArgs>
    T& operator()(internal::visitor_index<I, T>, internal::variant_storage_base<trivial, TArgs...>& vs)
    {
        return get<I>(vs);
    }
};


// NOTE: Using regular functions for F&& style functors doesn't
// seem to work, perhaps in particular due to class T parameter?
struct destroyer_functor
{
    template <unsigned I, class T>
    bool operator()(visitor_instance<I, T> vi, unsigned index)
    {
        if(I != index) return false;

        vi.value.~T();

        return true;
    }
};



template <bool trivial, class ...Types>
struct variant_storage_base : variant_storage_tag
{
    using size_type = std::size_t;
    typedef variant_storage_base<trivial, Types...> this_type;

    static constexpr bool is_trivial = trivial;

    using visitor = variadic::visitor<Types...>;

    template <class TEval>
    using selector = variadic::selector<TEval, Types...>;

    template <class T>
    using ensure_type_t = typename ensure_type<T, Types...>::type;

    template <int I>
    using type_at_index = estd::internal::type_at_index<I, Types...>;

    template <int I>
    using pointer_at_index = add_pointer_t<type_at_index<I>>;

    template <int I>
    using const_pointer_at_index = add_pointer_t<const type_at_index<I>>;

    template <class T>
    using constructable_selector = selector<internal::constructable_selector<T> >;

    using is_copy_constructible_selector =
        selector<internal::is_copy_constructible_selector>;

    struct copying_constructor_functor
    {
        template <unsigned I, class T_i, class enabled = enable_if_t<!is_copy_constructible<T_i>::value> >
        constexpr bool operator()(visitor_index<I, T_i>, this_type& v, const this_type& copy_from, const unsigned index) const
        {
            return false;
        }

        template <unsigned I, class T_i, class enabled = enable_if_t<is_copy_constructible<T_i>::value> >
        bool operator()(visitor_index<I, T_i>, this_type& v, const this_type& copy_from, const unsigned index)
        {
            if(index != I) return false;

            v.emplace<I>(internal::get<I>(copy_from));
            return true;
        }

        template <unsigned I, class T_i>
        bool operator()(visitor_instance<I, T_i> vi, const this_type& copy_from, const size_type index)
        {
            if(index != I) return false;

            new (&vi.value) T_i(internal::get<I>(copy_from));

            return true;
        }
    };



private:
    union
    {
        variant_union<trivial, Types...> storage;
        monostate dummy;
    };

public:
    variant_storage_base() = default;

    template <unsigned index, class ...TArgs>
    constexpr variant_storage_base(in_place_index_t<index>, TArgs&&...args) :
        dummy{
            construct_at<type_at_index<index>>
                (storage.raw, std::forward<TArgs>(args)...)}
    {
    }

    template <unsigned index>
    pointer_at_index<index> get()
    {
        return (pointer_at_index<index>) storage.raw;
    }

    template <unsigned index>
    const_pointer_at_index<index> get() const
    {
        return (const_pointer_at_index<index>) storage.raw;
    }

    template <unsigned index>
    void destruct()
    {
        typedef type_at_index<index> t;
        ((t*) storage.raw)->~t();
    }

    void destroy(unsigned index)
    {
        visit_instance(destroyer_functor{}, nullptr, index);
    }

    template <class T, class ...TArgs>
    T* emplace(TArgs&&...args)
    {
        T* const t = get<T>();
        new (t) T(std::forward<TArgs>(args)...);
        return t;
    }

    template <unsigned I, class ...TArgs>
    pointer_at_index<I> emplace(TArgs&&...args)
    {
        typedef type_at_index<I> T_i;
        return emplace<T_i>(std::forward<TArgs>(args)...);
    }


    template <class T>
    ensure_type_t<T>* get() { return (T*) storage.raw; }

    template <class T>
    constexpr ensure_type_t<T>* get() const { return (T*) storage.raw; }

    /*
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
    } */

    /*
    template <int I, typename F>
    static constexpr size_type visit(F&&) { return variant_npos(); }

    template <int I, typename F, class T, class... TArgs>
    size_type visit(F&& f)
    {
        if(f(get<T>())) return I;
        return visit<I + 1, F, TArgs...>(std::forward<F>(f));
    } */

    // DEBT: visit_instance auto adds 'this', visit doesn't
    template <typename F, class ...TArgs>
    monostate visit_instance(F&& f, size_type* index, TArgs&&...args)
    {
        int i = visitor::visit_instance(std::forward<F>(f),
            variant_storage_getter_functor{},
            *this,
            std::forward<TArgs>(args)...);

        if(index != nullptr) *index = (std::size_t)i;
        return {};
    }

    template <typename F, class ...TArgs>
    static monostate visit(F&& f, size_type* index, TArgs&&...args)
    {
        int i = visitor::visit(std::forward<F>(f),
                std::forward<TArgs>(args)...);

        if(index != nullptr) *index = (std::size_t)i;
        return {};
    }

    byte* raw() { return storage.raw; }
    const byte* raw() const { return storage.raw; }

    // DEBT: unsigned/signed int needs to be ironed out here, mismatch is gonna
    // continue to be a thorn in our side otherwise
    template <class F, class ...TArgs>
    variant_storage_base(in_place_visit_t, F&& f, size_type* index, TArgs&&...args) :
        dummy{visit(std::forward<F>(f), index, *this, std::forward<TArgs>(args)...)}
    {}

    constexpr variant_storage_base(const variant_storage_base& copy_from, size_type index) :
        //dummy{visit_instance(copying_constructor_functor{}, nullptr, copy_from, index)}
        dummy{visit(copying_constructor_functor{}, nullptr, *this, copy_from, index)}
    {}
};



template <class T>
struct converting_constructor_functor
{
    T& t;

    template <class T_i>
    constexpr bool operator()(T_i*) const { return false; }

    template <class T_i, class enabled = enable_if_t<estd::is_constructible<T_i, T>::value> >
    bool operator()(T_i* t_i)
    {
        new (t_i) T_i(std::forward<T>(t));
        return true;
    }
};


struct converting_constructor_functor2
{
    template <unsigned I, class TVariant, class T>
    constexpr bool operator()(in_place_index_t<I>, TVariant, T&&) const { return false; }

    template <unsigned I, class T_i, class TVariant, class T, class enabled = enable_if_t<estd::is_constructible<T_i, T>::value> >
    bool operator()(visitor_index<I, T_i>, TVariant& v, T&& t)
    {
        new (v.template get<I>()) T_i(std::forward<T>(t));
        return true;
    }
};


template <class ...Types>
class variant : public variant_storage<Types...>
{
    using base_type = variant_storage<Types...>;
    using size_type = typename base_type::size_type;

    struct moving_constructor_functor
    {
        template <unsigned I, class T_i>
        bool operator()(visitor_index<I, T_i>, base_type& v, variant&& move_from)
        {
            if(move_from.index() != I) return false;

            T_i& t = *move_from.template get<I>();

            new (v.template get<I>()) T_i(std::move(t));

            // Now that it's moved, we opt to immediately destroy it.  Alternatively,
            // we could let the moved object linger and keep the index set, thus running
            // the destructor more "naturally".
            // pros: immediate dtor suggests more immediate availability of resources
            // cons: a tiny bit more time and code to check/assign index
            t.~T_i();

            // NOTE: Not setting index_ here becuase we need it to stay put for
            // intiialization list to pick it up

            return true;
        }
    };

    struct assignment_functor
    {
        template <unsigned I, class T_i,
            class enabled = enable_if_t<is_copy_assignable<T_i>::value> >
        bool operator()(visitor_index<I, T_i>, base_type& v, const variant& assign_from)
        {
            if(assign_from.index() != I) return false;

            //(v.template get<I>())->operator =(* assign_from.template get<I>());
            (*v.template get<I>()) = * assign_from.template get<I>();
            return true;
        }

        // NOTE: This whole direct-init flavor doesn't seem to conform, so this might
        // go away.  Convenient though for non trivial types which don't have an assignment
        // operator.  Probably should feature flag it
        template <unsigned I, class T_i,
            class enabled = enable_if_t<!is_copy_assignable<T_i>::value> >
        bool operator()(visitor_index<I, T_i>, variant& v, const variant& copy_from, bool = true)
        {
            if(copy_from.index() != I) return false;

            T_i* obj = v.template get<I>();

            // DEBT: Technically would be more efficient to run the dtor
            // directly and not abort when index is found - however, that
            // breaks our burgeoning paradigm, so holding off
            v.destroy_if_valid();

            new (obj) T_i(* copy_from.template get<I>());
            return true;
        }
    };


    struct converting_assignment_functor
    {
        template <unsigned I, class T_i, class T, class enabled =
            enable_if_t<is_constructible<T_i, T>::value> >
        bool operator()(visitor_instance<I, T_i> vi, size_type index, T&& t)
        {
            return false;
        }
    };

    size_type index_;

    constexpr bool valid() const { return index_ != variant_npos(); }

    // IDEA: Consider a feature flag to track the particular destructor of interest
    // via a function pointer instead of this visitor pattern.  Would be a pretty specific
    // speed vs size edge case
    void destroy_if_valid()
    {
        if(valid()) base_type::destroy(index_);
    }

    template <size_t I>
    using variant_alternative_t = estd::variant_alternative_t<I, variant>;

public:
    template <class T>
    using index_of_type = estd::internal::select_type<T, Types...>;

    constexpr variant() :
        base_type(in_place_index_t<0>{}),
        index_{0}
    {}

    constexpr variant(const variant& copy_from)
#if __cpp_concepts
        requires(base_type::is_copy_constructible_selector::all)
#endif
        :
        base_type(copy_from, copy_from.index()),
        index_{copy_from.index()}
    {

    }


    template <unsigned index, class ...TArgs>
    constexpr explicit variant(in_place_index_t<index>, TArgs&&...args) :
        base_type(in_place_index_t<index>{}, std::forward<TArgs>(args)...),
        index_{index}
    {}

    template <class T, class ...TArgs>
    constexpr explicit variant(in_place_type_t<T>, TArgs&&...args) :
        base_type(
            in_place_index_t<index_of_type<T>::index>{},
            std::forward<TArgs>(args)...),
        index_{index_of_type<T>::index}
    {}

#if __cpp_concepts
    template <class T>
    constexpr variant(T&& t)
        requires(
            !is_same_v<remove_cvref_t<T>, variant> &&
            !is_base_of_v<in_place_tag, remove_cvref_t<T>>
            ) :
#else
    template <class T, class enabled =
        enable_if_t<
            !is_same<remove_cvref_t<T>, variant>::value &&
            !is_base_of<in_place_tag, remove_cvref_t<T>>::value> >
    constexpr variant(T&& t) :
#endif
        base_type(in_place_visit_t{}, converting_constructor_functor2{},
                  &index_,
                  std::forward<T>(t))
    {

    }

    constexpr bool valueless_by_exception() const
    {
        return index_ == variant_npos();
    }

    ~variant()
    {
        //apply<sizeof...(Types) - 1>(destroyer_functor{});
        //apply<sizeof...(Types) - 1>(&destroy);
        destroy_if_valid();
    }

    variant(variant&& move_from) noexcept :
        base_type(in_place_visit_t{}, moving_constructor_functor{}, &index_, std::move(move_from)),
        index_{move_from.index()}
    {
        // moving_constructo_functor will do the bulk of the move, but leaves
        // index clearing for constructor
        // DEBT: May need extra legwork in cases where move constructor didn't find the index
        move_from.index_ = variant_npos();
    }

    template <unsigned I, class T_j, class T,
        class enabled = enable_if_t<
            is_nothrow_constructible<T_j, T>::value ||
            !is_nothrow_move_constructible<T_j>::value> >
    void assignment_emplace_helper(T&& t, bool = true)
    {
        // DEBT: Make this emplace<index> instead as per spec.
        // Should be OK as type for now though
        emplace<T_j>(std::forward<T>(t));
    }

    template <unsigned I, class T_j, class T,
        class enabled = enable_if_t<
                !(is_nothrow_constructible<T_j, T>::value ||
                !is_nothrow_move_constructible<T_j>::value)> >
    void assignment_emplace_helper(T&& t)
    {
        // DEBT: Make this emplace<index> instead as per spec.
        // Should be OK as type for now though
        emplace<T_j>(T_j(std::forward<T>(t)));
    }

    template <class T_j, class T, class enabled = enable_if_t<is_assignable<T_j&, T>::value> >
    void assignment_helper(T&& t)
    {
        *base_type::template get<T_j>() = std::forward<T>(t);
    }

    template <class T_j, class T, class enabled = enable_if_t<!is_assignable<T_j&, T>::value> >
    void assignment_helper(T&& t, bool = true)
    {
        // DEBT: std variant spec doesn't appear to handle 'emplace' for like-indexed assignment,
        // but we do.  Consider feature-flagging
        // DEBT: Fix up dummy index
        assignment_emplace_helper<0, T_j, T>(std::forward<T>(t));
    }

    template <
        class T,
        class enabled = enable_if_t<!is_base_of<variant_storage_tag, remove_cvref_t<T> >::value> >
    variant& operator=(T&& t)
    {
        typedef typename base_type::template constructable_selector<T> selector;
        typedef typename selector::selected_type T_j;

        // DEBT: Multiple constructable T_j could be found.  Spec implies this is not
        // allowed.  However, in our case, we choose the first one.  Find a tighter
        // spec to indicate what we should really do here and consider a feature flag

        if(selector::selected == index_)
        {
            assignment_helper<T_j>(std::forward<T>(t));
        }
        else
        {
            assignment_emplace_helper<selector::selected, T_j>(std::forward<T>(t));
        }

        return *this;
    }

    // DEBT: Needs more work, but coming along
    variant& operator=(const variant& rhs)
    {
        if(rhs.valueless_by_exception())
        {
            destroy_if_valid();
            index_ = variant_npos();
        }
        else
        {
            base_type::visit(assignment_functor{}, &index_, *this, rhs);
        }

        return *this;
    }

    constexpr size_type index() const { return index_; }

    template <class T, class ...TArgs,
        class enabled = enable_if_t<is_constructible<T, TArgs...>::value> >
    T& emplace(TArgs&&...args)
    {
        destroy_if_valid();

        //typedef typename base_type::visitor::visit_struct<constructable_selector<T> > finder_type;
        //typedef typename finder_type::selected_type T_i;

        // Unset index just in case expection is thrown during construction
        index_ = variant_npos();

        T* const v = base_type::template emplace<T>(std::forward<TArgs>(args)...);

        index_ = index_of_type<T>::index;

        return *v;
    }

    template <unsigned I, class... TArgs>
    variant_alternative_t<I>& emplace(TArgs&&...args)
    {
        typedef variant_alternative_t<I> T_i;

        return emplace<T_i>(std::forward<TArgs>(args)...);
    }
};

}}
#endif
