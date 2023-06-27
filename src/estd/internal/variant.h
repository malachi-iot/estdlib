#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "fwd/variant.h"

#include "variant/storage.h"

#if !FEATURE_ESTD_VARIANT_PERMISSIVE_ASSIGNMENT
#error variant: strict assignment not supported yet
#endif

#if !FEATURE_ESTD_VARIANT_EAGER_DESTRUCT
#error variant: only eager destruct mode supported
#endif

#if FEATURE_STD_CSTDLIB
#include <cstdlib>
#else
#include "stdlib.h"
#endif

namespace estd {

#if __cpp_variadic_templates

namespace internal {

template <size_t index, class ...Types>
constexpr bool holds_index(const variant<Types...>* vs)
{
    return vs != nullptr && vs->index() == index;
}


template <class T, class ...Types>
constexpr bool holds_type(const variant<Types...>* vs)
{
    typedef select_type<T, Types...> selected;

    // NOTE: size() check is redundant, because compile time check for 'first()' below fails
    // if no items are present.  This does not clash with spec, which indicates
    // this is "ill-formed if T is not a unique element"
    return selected::size() != 0 &&
           (vs != nullptr && vs->index() == selected::first::index);
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

template <class ...Types>
class variant : protected variant_storage<Types...>
{
    using base_type = variant_storage<Types...>;
    using typename base_type::size_type;
    using typename base_type::visitor;

    struct moving_constructor_functor
    {
        template <size_t I, class T_i>
        bool operator()(variadic::visitor_index<I, T_i>, base_type& v, variant&& move_from)
        {
            if(move_from.index() != I) return false;

            v.template move<I>(std::move(move_from));

            // Now that it's moved, we opt to immediately destroy it.  Alternatively,
            // we could let the moved object linger and keep the index set, thus running
            // the destructor more "naturally".
            // pros: immediate dtor suggests more immediate availability of resources
            // cons: a tiny bit more time and code to check/assign index
            move_from.template destroy<I>();

            // NOTE: Not setting index_ here becuase we need it to stay put for
            // initialization list to pick it up

            return true;
        }
    };

    // NOTE: Have to remove const since adding const to T_i is needed when pulling instance
    // from const variant
    struct assignment_functor
    {
        template <size_t I, class T_i,
            class enabled = enable_if_t<is_copy_assignable<remove_const_t<T_i>>::value> >
        bool operator()(variadic::visitor_instance<I, T_i> vi, variant& v, const variant& assign_from)
        {
            size_type index = v.index();
            v.template assign_or_init<I>(&index, vi.value);
            return true;
        }


        template <size_t I, class T_i,
                 class enabled = enable_if_t<is_move_assignable<remove_const_t<T_i>>::value> >
        bool operator()(variadic::visitor_instance<I, T_i> vi, variant& v, variant&& assign_from)
        {
            size_type index = v.index();
            v.template assign_or_init<I>(&index, std::move(vi.value));
            return true;
        }

        template <size_t I, class T_i,
            class enabled = enable_if_t<!is_copy_assignable<remove_const_t<T_i>>::value> >
        bool operator()(variadic::visitor_index<I, T_i>, variant& v, const variant& copy_from, bool = true)
        {
            // DEBT: Technically would be more efficient to run the dtor
            // directly and not abort when index is found - however, that
            // breaks our burgeoning paradigm, so holding off
            v.destroy_if_valid();

            v.template copy<I>(copy_from);

            return true;
        }


        // NOTE: See comments in above copy_from flavor
        template <size_t I, class T_i,
                 class enabled = enable_if_t<!is_move_assignable<remove_const_t<T_i>>::value> >
        bool operator()(variadic::visitor_index<I, T_i>, variant& v, variant&& move_from, bool = true)
        {
            v.destroy_if_valid();

            v.template move<I>(std::move(move_from));

#if FEATURE_ESTD_VARIANT_EAGER_DESTRUCT
            // remember, our variant destroys as soon as we determine transition to
            // valueless state - once valueless, we lose ability to identify who
            // needs destruction
            move_from.template destroy<I>();
            move_from.index_ = variant_npos();
#endif

            return true;
        }
    };


    struct converting_assignment_functor
    {
        template <size_t I, class T_i, class T, class enabled =
            enable_if_t<is_constructible<T_i, T>::value> >
        bool operator()(variadic::visitor_instance<I, T_i> vi, size_type index, T&& t)
        {
            return false;
        }
    };

    size_type index_;

    constexpr bool valid() const { return index_ != variant_npos(); }

    // IDEA: Consider a feature flag to track the particular destructor of interest
    // via a function pointer instead of this visitor pattern.  Would be a pretty specific
    // speed vs size edge case
    bool destroy_if_valid()
    {
        //if(valid()) // base destoy innately checks for valid index
        return base_type::destroy(index_);
    }

    template <size_t I>
    using variant_alternative_t = estd::variant_alternative_t<I, variant>;

    // These friends are necessary because we plan to make variant_storage base
    // protected
    template <int index, class ...Types2>
    friend type_at_index<index, Types2...>* get_ll(variant<Types2...>& vs) noexcept;

    template <int index, class ...Types2>
    friend constexpr const type_at_index<index, Types2...>* get_ll(const variant<Types2...>& vs) noexcept;

    // DEBT: I think we can isolate this into storage_getter and get rid of
    // the friend
    friend struct variant_storage_getter_functor;

    // DEBT: I'd like to phase this one out if we can
    friend struct variadic::visitor<Types...>;

public:
    template <class T>
    using select_type = typename estd::internal::select_type<T, Types...>::first;

    template <class F, class ...TArgs>
    size_type visit(F&& f, TArgs&&...args)
    {
        int selected = base_type::visit_instance(
            std::forward<F>(f),
            std::forward<TArgs>(args)...);
        return selected == -1 ? variant_npos() : selected;
    }

    constexpr variant() :
        base_type(in_place_index_t<0>{}),
        index_{0}
    {}

    constexpr variant(const variant& copy_from)
#if __cpp_concepts
        requires(base_type::is_copy_constructible_selector::size() == sizeof...(Types))
#endif
        :
        base_type(copy_from, copy_from.index()),
        index_{copy_from.index()}
    {

    }


    template <size_t index, class ...TArgs>
    constexpr explicit variant(in_place_index_t<index>, TArgs&&...args) :
        base_type(in_place_index_t<index>{}, std::forward<TArgs>(args)...),
        index_{index}
    {}

    template <class T, class ...TArgs>
    constexpr explicit variant(in_place_type_t<T>, TArgs&&...args) :
        base_type(
            in_place_index_t<select_type<T>::index>{},
            std::forward<TArgs>(args)...),
        index_{select_type<T>::index}
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

    template <
        class T,
        class enabled = enable_if_t<!is_base_of<variant_storage_tag, remove_cvref_t<T> >::value> >
    variant& operator=(T&& t)
    {
        base_type::assign_or_init(&index_, std::forward<T>(t));

        return *this;
    }

    template <class F, class ...TArgs>
    size_type visit_index(F&& f, TArgs&&...args) const
    {
        int i = visitor::visit(typename base_type::index_visitor{},
            index_,
            *this,
            std::forward<F>(f),
            std::forward<TArgs>(args)...);

        return i == -1 ? variant_npos() : i;
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
            //base_type& _rhs = rhs;
            // DEBT: visit_index does more legwork than is needed, grabbing
            // actual value via visit_instance.  Optimize that out
            index_ = rhs.visit_index(assignment_functor{}, *this, rhs);
        }

        return *this;
    }


    variant& operator=(variant&& rhs)
    {
        if(rhs.valueless_by_exception())
        {
            destroy_if_valid();
            index_ = variant_npos();
        }
        else
        {
            index_ = rhs.visit_index(assignment_functor{}, *this, std::move(rhs));
        }

        return *this;
    }

    constexpr size_type index() const { return index_; }

    template <class T, class ...TArgs,
        class enabled = enable_if_t<is_constructible<T, TArgs...>::value> >
    T& emplace(TArgs&&...args)
    {
        destroy_if_valid();

        // Unset index just in case exception is thrown during construction
        index_ = variant_npos();

        T* const v = base_type::template emplace<T>(std::forward<TArgs>(args)...);

        index_ = select_type<T>::index;

        return *v;
    }

    template <unsigned I, class... TArgs>
    variant_alternative_t<I>& emplace(TArgs&&...args)
    {
        typedef variant_alternative_t<I> T_i;

        return emplace<T_i>(std::forward<TArgs>(args)...);
    }
};

}

#else

namespace internal {


}

#endif
}
