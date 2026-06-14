#pragma once

#include "core.h"
#include "accessors.h"

#include "../../cstdlib.h"
#include "union.h"

namespace estd {

template <class... Types>
struct variant_size<internal::variant_storage<Types...> > : variadic::size<Types...> {};

template <size_t I, class... Types>
struct variant_alternative<I, internal::variant_storage<Types...> > :
    type_identity<internal::type_at_index<I, Types...>> { };

namespace internal {

#if FEATURE_ESTD_VARIANT_PERMISSIVE_ASSIGNMENT
template <class T, class U>
using is_variant_assignable = is_constructible<T, U>;
#else
template <class T, class U>
using is_variant_assignable = bool_constant<
    is_constructible<T, U>::value &
    is_assignable<T&, U>::value>;
#endif

// Very similar to c++20 flavor, but importantly returns monostate instead
// FIX: A lie - c++20 flavor appears to be a loophole to permit otherwise-prohibited constexpr placement new
// DEBT: Document why that's wanted
template <class T, class ...Args>
inline ESTD_CPP_CONSTEXPR(17) static monostate construct_at(void* placement, Args&&...args)
{
    new (placement) T(std::forward<Args>(args)...);
    return {};
}


struct variant_storage_getter_functor
{
    template <size_t I, class T, class ...Types>
    ESTD_CPP_CONSTEXPR(14) T& operator()(
        variadic::visitor_index<I, T>, internal::variant_storage<Types...>& vs) const
    {
        return get<I>(vs);
    }

    template <size_t I, class T, class ...Types>
    ESTD_CPP_CONSTEXPR(14) T& operator()(variadic::visitor_index<I, T>, variant<Types...>& vs) const
    {
        // DEBT: A bit of a cheat
        return *get_ll<I>(vs);
    }
};


// NOTE: Using regular functions for F&& style functors doesn't
// seem to work, perhaps in particular due to class T parameter?
struct destroyer_functor
{
    template <size_t I, class T>
    bool operator()(variadic::visitor_instance<I, T> vi)
    {
        vi.value.~T();

        return true;
    }
};



template <bool trivial, class ...Types>
struct variant_storage_base : variant_storage_tag
{
    using size_type = std::size_t;
    using this_type = variant_storage_base<trivial, Types...>;
    using types = variadic::types<Types...>;

    struct index_visitor
    {
        template <size_t I, class T, class F, class ...TArgs>
        constexpr bool operator()(variadic::visitor_index<I, T>, size_type index,
            this_type& this_, F&& f, TArgs&&...args) const
        {
            return I == index ?
                f(variadic::visitor_instance<I, T>{*this_.get<I>()},
                    std::forward<TArgs>(args)...) :
                false;
        }


        template <size_t I, class T, class F, class ...TArgs>
        constexpr bool operator()(variadic::visitor_index<I, T>, size_type index,
            const this_type& this_, F&& f, TArgs&&...args) const
        {
            return I == index ?
                f(variadic::visitor_instance<I, const T>{*this_.get<I>()},
                    std::forward<TArgs>(args)...) :
                false;
        }

        /*
        template <size_t I, class T, class F, class ...TArgs>
        constexpr bool operator()(variadic::visitor_index<I, T>, size_type index,
            F&& f, TArgs&&...args) const
        {
            return I == index ?
                f(variadic::visitor_index<I, T>{},
                    std::forward<TArgs>(args)...) :
                false;
        }   */
    };


    struct getter_functor2
    {
        template <size_t I, class T, class F, class ...Args>
        constexpr bool operator()(variadic::visitor_index<I, T>, this_type& vs, F&& f, Args&&...args) const
        {
            return f(variadic::visitor_instance<I, T>{*vs.get<I>()}, std::forward<Args>(args)...);
        }

        template <size_t I, class T, class F, class ...Args>
        constexpr bool operator()(variadic::visitor_index<I, T>, const this_type& vs, F&& f, Args&&...args) const
        {
            return f(variadic::visitor_instance<I, T>{*vs.get<I>()}, std::forward<Args>(args)...);
        }
    };

    static constexpr bool is_trivial = trivial;

    using visitor = variadic::type_visitor<Types...>;

    template <class F, class ...TArgs>
    int visit_index(size_type index, F&& f, TArgs&&...args)
    {
        return visitor::visit(index_visitor{}, index, *this,
            std::forward<F>(f),
            std::forward<TArgs>(args)...);
    }

    template <class F, class ...TArgs>
    constexpr int visit_index(size_type index, F&& f, TArgs&&...args) const
    {
        return visitor::visit(index_visitor{}, index, *this,
            std::forward<F>(f),
            std::forward<TArgs>(args)...);
    }

    template <class Eval>
    using selector = variadic::v1::selector<Eval, Types...>;

    template <class T>
    using ensure_type_t = typename ensure_type<T, Types...>::type;

    template <class T>
    using ensure_pointer = add_pointer_t<ensure_type_t<T>>;

    template <size_t I>
    using type_at_index = estd::internal::type_at_index<I, Types...>;

    template <size_t I>
    using pointer_at_index = add_pointer_t<type_at_index<I>>;

    template <size_t I>
    using const_pointer_at_index = add_pointer_t<const type_at_index<I>>;

    template <class... Types2>
    using is_constructible_selector = selector<
        internal::constructible_selector<Types2...>>;

    template <class... Types2>
    using is_convertible_selector = selector<
        internal::converting_selector<Types2...>>;

    using is_copy_constructible_selector =
        selector<internal::is_copy_constructible_selector>;

    struct copying_constructor_functor
    {
        template <size_t I, class T_i,
            enable_if_t<!is_copy_constructible<T_i>::value, bool> = true>
        constexpr bool operator()(variadic::type<I, T_i>, this_type& v, const this_type& copy_from, const unsigned index) const
        {
            return false;
        }

        template <size_t I, class T_i,
            enable_if_t<is_copy_constructible<T_i>::value, bool> = true>
        bool operator()(variadic::type<I, T_i>, this_type& v, const this_type& copy_from, const unsigned index)
        {
            if(index != I) return false;

            v.template emplace<I>(internal::get<I>(copy_from));
            return true;
        }

        template <size_t I, class T_i>
        bool operator()(variadic::instance<I, T_i> vi, const this_type& copy_from, const size_type index)
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

        // constexpr constructors need to init some guy during construct_at.  Note that
        // construct_at is only c++17 friendly at the moment, so the notion is either DEBT
        // or flawed
        monostate dummy;
    };

protected:
    byte* raw() { return storage.raw; }
    const byte* raw() const { return storage.raw; }

public:
    variant_storage_base() = default;

    template <size_t index, class ...Args>
    constexpr explicit variant_storage_base(in_place_index_t<index>, Args&&...args) :
        dummy{
            construct_at<type_at_index<index>>
                (storage.raw, std::forward<Args>(args)...)}
    {
    }

    template <size_t index, class ...Args>
    constexpr explicit variant_storage_base(in_place_conditional_t<index>, bool condition, Args&&...args) :
        dummy{
            condition ?
                construct_at<type_at_index<index>>
                    (storage.raw, std::forward<Args>(args)...) :
                monostate{}}
    {
    }


    template <size_t index>
    pointer_at_index<index> get()
    {
        return (pointer_at_index<index>) storage.raw;       // NOLINT
    }

    template <size_t index>
    constexpr const_pointer_at_index<index> get() const
    {
        return (const_pointer_at_index<index>) storage.raw; // NOLINT
    }

    template <size_t index>
    void destroy()
    {
        typedef type_at_index<index> t;
        get<index>()->~t();
    }

    constexpr bool destroy(unsigned index) const
    {
        return visit_index(index, destroyer_functor{}) != -1;
    }

    template <class T, class ...Args>
    constexpr ensure_pointer<T> emplace(Args&&...args)
    {
        return new (storage.raw) T(std::forward<Args>(args)...);
    }

    template <size_t I, class ...Args>
    constexpr pointer_at_index<I> emplace(Args&&...args)
    {
        using T_i = type_at_index<I>;
        return emplace<T_i>(std::forward<Args>(args)...);
    }


    template <class T>
    ESTD_CPP_CONSTEXPR(14) ensure_pointer<T> get() { return (T*) storage.raw; }

    template <class T>
    constexpr add_pointer_t<const ensure_type_t<T>> get() const
    {
        return reinterpret_cast<add_pointer_t<const T>>(storage.raw);
    }

    // Same operation as operator=, but more explicit since
    // type safety is up to programmer
    template <size_t I>
    constexpr type_at_index<I>& assign(const this_type& assign_from)
    {
        return *get<I>() = *assign_from.get<I>();
    }

    template <size_t I>
    constexpr type_at_index<I>& assign(this_type&& assign_from)
    {
        // DEBT: Do proper forwarding for assign_from
        return *get<I>() = std::move(*assign_from.get<I>());
    }

    template <size_t I>
    constexpr type_at_index<I>& assign(const type_at_index<I>& assign_from)
    {
        return *get<I>() = assign_from;
    }

    template <size_t I>
    constexpr type_at_index<I>& assign(type_at_index<I>&& assign_from)
    {
        return *get<I>() = std::move(assign_from);
    }

    // Same operation as copy constructor, but more explicit since
    // type safety is up to programmer
    template <size_t I>
    constexpr pointer_at_index<I> copy(const this_type& copy_from)
    {
        return new (storage.raw) type_at_index<I> (*copy_from.get<I>());
    }

    // Same operation as move constructor, but more explicit since
    // type safety is up to programmer
    template <size_t I>
    void move(this_type&& move_from)
    {
        new (storage.raw) type_at_index<I> (std::move(*move_from.get<I>()));
    }

    template <size_t I, class T_i, class T,
        enable_if_t<
            !is_convertible<T, T_i>::value, bool> = true>
    static constexpr pointer_at_index<I> assignment_emplace_helper(T&&, long = 0)
    {
#if FEATURE_ESTD_VARIANT_STRICT_CONVERSION
        std::abort();
#endif
        return {};
    }


    // DEBT: We often don't get here because is_nothrow_constructible doesn't like a const T&
    // Not sure yet if that's "proper behavior" as indicated by variant docs, but seems to be
    template <size_t I, class T_i, class T,
        enable_if_t<
            is_convertible<T, T_i>::value &&
            (is_nothrow_constructible<T_i, T>::value ||
            !is_nothrow_move_constructible<T_i>::value), bool> = true>
    constexpr pointer_at_index<I> assignment_emplace_helper(T&& t, bool = true)
    {
        return emplace<I>(std::forward<T>(t));
    }

    template <size_t I, class T_i, class T,
        enable_if_t<
            is_convertible<T, T_i>::value &&
            !(is_nothrow_constructible<T_i, T>::value ||
            !is_nothrow_move_constructible<T_i>::value), bool> = true>
    constexpr pointer_at_index<I> assignment_emplace_helper(T&& t)
    {
        return emplace<I>(T_i(std::forward<T>(t)));
    }

    template <size_t I, class T_i, class T,
        enable_if_t<is_assignable<T_i&, T>::value, bool> = true>
    constexpr type_at_index<I>& assignment_helper(T&& t)
    {
        //assign<I>(std::forward<T>(t));
        return *get<I>() = std::forward<T>(t);
    }

    template <size_t I, class T_i, class T,
        enable_if_t<!is_assignable<T_i&, T>::value, bool> = true>
    ESTD_CPP_CONSTEXPR(14) type_at_index<I>& assignment_helper(T&& t, bool = true)
    {
        // DEBT: std variant spec doesn't appear to handle 'emplace' for like-indexed assignment,
        // but we do.  Consider feature-flagging
        destroy<I>();
        return *assignment_emplace_helper<I, T_i, T>(std::forward<T>(t));
    }

    // 14JUN26 MB DEBT: Have a closer look at this guy, this kind of enable_if is sometimes flaky
    template <size_t I, class T_i, class T,
        enable_if_t<
            is_convertible<T, T_i>::value &&
            estd::is_trivial<T_i>::value, bool> = true>
    constexpr type_at_index<I>& direct_init_helper(T&& t)
    {
        return *get<I>() = std::forward<T>(t);
    }

    template <size_t I, class T_i, class T,
        enable_if_t<
            estd::is_trivial<T_i>::value &&
            !is_convertible<T, T_i>::value, bool> = true>
    void direct_init_helper(T&& t, long = 0)
    {
        //*get<I>() = std::forward<T>(t);
        //static_assert(is_convertible<T, T_i>::value, "ERR");
#if FEATURE_ESTD_VARIANT_STRICT_CONVERSION
        std::abort();
#endif
    }

    template <size_t I, class T_i, class T,
        enable_if_t<
            is_convertible<T, T_i>::value &&
#if FEATURE_STD_TYPE_TRAITS
            !std::is_trivially_constructible<T_i>::value, bool> = true>
#elif FEATURE_ESTD_IS_TRIVIAL
            !estd::is_trivial<T_i>::value, bool> = true>
#endif
    void direct_init_helper(T&& t, bool = true)
    {
        assignment_emplace_helper<I, T_i>(std::forward<T>(t));
    }


    // index = index of variant tracked in 'this' - since it's a compile time constant,
    // this is most useful for when match toggles between two possibilities (such as
    // 'expected')
    /// @param match are we currently tracking 'I'
    // DEBT: Operates with variant sensibilities, specifically in that as per
    // https://en.cppreference.com/w/cpp/utility/variant/operator%3D will assign to a
    // temporary copy of T_j if T_j has a noexcept move constructor or a non-noexcept copy constructor.
    template <size_t I, size_t index, class U>
    ESTD_CPP_CONSTEXPR(14) void assign_or_init(bool match, U&& u)
    {
        using T_j = type_at_index<I>;

        // Are we tracking 'I'?  If so, assign over it
        if(match)
        {
            assignment_helper<I, T_j>(std::forward<U>(u));
        }
        else
        {
            // ... if not, destroy what we're tracking (if any) and
            // do a direct initialization
            destroy<index>();
            direct_init_helper<I, T_j>(std::forward<U>(u));
        }
    }

    // index = index of variant tracked in 'this'
    template <size_t I, class U>
    ESTD_CPP_CONSTEXPR(14) void assign_or_init(size_type* index, U&& u)
    {
        using T_j = type_at_index<I>;

        // DEBT: A static assert to determine whether U is convertible/
        // assignable to T_j would be useful here
#if __cpp_static_assert
#endif

        // Are we tracking the exact type being assigned?
        if(*index == I)
        {
            assignment_helper<I, T_j>(std::forward<U>(u));
        }
        else
        {
            // ... if not, destroy what we're tracking (if any) and
            // do a direct initialization
            destroy(*index);
            direct_init_helper<I, T_j>(std::forward<U>(u));
            *index = I;
        }
    }

    // DEBT: Workaround for AVR 'is_constructible' reporting true
    // way too often (theory is -fpermissive is responsible)
#ifdef __AVR__
    template <class U>
    using assign_or_init_selector = is_convertible_selector<U>;
#else
    template <class U>
    using assign_or_init_selector = is_constructible_selector<U>;
#endif

    // Assign or direct initialize, depending on whether index matches
    // and what is matched to is assignable vs constructible
    // NOTE: This will assign the first one it finds; however, multiple
    // constructible technically is undefined behavior so do not rely on this
    template <class U>
    ESTD_CPP_CONSTEXPR(14) void assign_or_init(size_type* index, U&& u)
    {
        using selected = typename assign_or_init_selector<U>::first;

        assign_or_init<selected::index>(index, std::forward<U>(u));
    }

    template <typename F, class ...Args>
    ESTD_CPP_CONSTEXPR(14) int visit_instance(F&& f, Args&&...args)
    {
        /*
        int i = visitor::visit_instance(std::forward<F>(f),
            variant_storage_getter_functor{},
            *this,
            std::forward<TArgs>(args)...); */
        int i = visitor::visit(getter_functor2{}, *this,
            std::forward<F>(f),
            std::forward<Args>(args)...);

        return i;
    }

    template <typename F, class ...Args>
    static monostate visit(F&& f, size_type* index, Args&&...args)
    {
        const size_type i = visitor::visit(std::forward<F>(f),
            std::forward<Args>(args)...);

        if(index != nullptr) *index = i;
        return {};
    }

    // DEBT: unsigned/signed int needs to be ironed out here, mismatch is gonna
    // continue to be a thorn in our side otherwise
    template <class F, class ...Args>
    variant_storage_base(in_place_visit_t, F&& f, size_type* index, Args&&...args) :
        dummy{visit(std::forward<F>(f), index, *this, std::forward<Args>(args)...)}
    {}

    constexpr variant_storage_base(const variant_storage_base& copy_from, size_type index) :
        //dummy{visit_instance(copying_constructor_functor{}, nullptr, copy_from, index)}
        dummy{visit(copying_constructor_functor{}, nullptr, *this, copy_from, index)}
    {}
};


template <class T>
class instance_storage : protected variant_storage_base<is_trivially_constructible<T>::value, T>
{
    using base_type = variant_storage_base<is_trivially_constructible<T>::value, T>;

    ESTD_CPP_STD_VALUE_TYPE(T)

public:
    // NOLINTBEGIN

    template <class ...Args>
    constexpr explicit instance_storage(in_place_t, Args&&...args) :
        base_type(in_place_index_t<0>{}, std::forward<Args>(args)...)
    {}

    instance_storage() = default;

    pointer get() { return base_type::template get<0>(); }
    constexpr const_pointer get() const { return base_type::template get<0>(); }

    constexpr const_pointer operator->() const { return get(); }

    template <class ...Args>
    pointer emplace(Args&&...args)
    {
        return base_type::template emplace<0>(std::forward<Args>(args)...);
    }

    void destroy() { return base_type::template destroy<0>(); }

    pointer assign(const_reference copy_from)
    {
        return base_type::template assign<0>(copy_from);
    }

    // NOLINTEND
};


template <class T>
struct converting_constructor_functor
{
    T& t;

    template <class T_i>
    constexpr bool operator()(T_i*) const { return false; }

    template <class T_i,
        enable_if_t<estd::is_constructible<T_i, T>::value, bool> = true>
    bool operator()(T_i* t_i)
    {
        new (t_i) T_i(std::forward<T>(t));
        return true;
    }
};


struct converting_constructor_functor2
{
    template <size_t I, class TVariant, class T>
    constexpr bool operator()(in_place_index_t<I>, TVariant, T&&) const { return false; }

    template <size_t I, class T_i, class TVariant, class T,
        enable_if_t<estd::is_constructible<T_i, T>::value, bool> = true>
    bool operator()(variadic::visitor_index<I, T_i>, TVariant& v, T&& t)
    {
        new (v.template get<I>()) T_i(std::forward<T>(t));
        return true;
    }
};

}

}
