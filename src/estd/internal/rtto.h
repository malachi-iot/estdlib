#pragma once

#include "../system_error.h"

#include "type_traits.h"
#include "utility.h"

// EXPERIMENTAL still and occupies ROM space even when not used
#ifndef FEATURE_ESTD_RTTO_GET_METADATA
#define FEATURE_ESTD_RTTO_GET_METADATA 0
#endif

// EXPERIMENTAL - retrieve invokable function pointer (caller must know exact signature, dangerous!)
// In theory saves space for fnptr2, we can rely on just utility fnptr rather than utility+exec.
// One wonders if a vtable is better at this point though
#ifndef FEATURE_ESTD_RTTO_GET_INVOKE
#define FEATURE_ESTD_RTTO_GET_INVOKE 1
#endif


namespace estd { namespace internal {

// DEBT: CREATE mode is experimental and is_default_constructible falls apart for forwarding constructors,
// resulting in compile time errors in that case when base class really isn't default constructible.
// Therefore we play some games here.
template <class T, class IsDefaultConstructible = std::is_trivially_default_constructible<T>>
struct rtto_traits
{
    using value_type = T;

    // DEBT: Switch these to estd variety for AVR compability
    using is_copy_constructible = std::is_copy_constructible<value_type>;
    using is_move_constructible = std::is_move_constructible<value_type>;
    using is_trivially_constructible = std::is_trivially_constructible<value_type>;
    //using is_constructible = std::is_default_constructible<value_type>;
    using is_constructible = IsDefaultConstructible;
#if __cpp_lib_is_swappable
    using is_swappable = std::is_swappable<value_type>;
#else
    using is_swappable = std::false_type;
#endif
};

struct rtto_modes
{
    enum modes
    {
        COPY,
        MOVE,
        MOVE_AND_DESTROY,
        DELETE,
        SIZE,

        // +++ EXPERIMENTAL
        CREATE,
        GET_METADATA,
        COPY_AND_SWAP,
        MOVE_AND_SWAP,
        GET_INVOKE,
        // --- EXPERIMENTAL
    };

    /// generally returns POSIX errno.h codes except for:
    /// SIZE which returns type size
    using utility_type = int (*)(modes, void*, int, void*);
};



constexpr int copy_to(rtto_modes::utility_type u, const void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::COPY, const_cast<void*>(src), sz, dest);
}

constexpr int move_to(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE, src, sz, dest);
}

constexpr int move_to_and_destroy(rtto_modes::utility_type u, void* src, void* dest, int sz = 0)
{
    return u(rtto_modes::MOVE_AND_DESTROY, src, sz, dest);
}

inline ESTD_CPP_CONSTEXPR(14) void destroy(rtto_modes::utility_type u, void* src)
{
    u(rtto_modes::DELETE, src, 0, nullptr);
}

constexpr int size(const rtto_modes::utility_type u)
{
    return u(rtto_modes::SIZE, nullptr, 0, nullptr);
}

template <class T, class Traits = rtto_traits<T>>
struct rtto;

struct rtto_base : rtto_modes
{
    // EXPERIMENTAL
    struct metadata
    {
        const int value_sz;
        bool copyable : 1;
        bool moveable : 1;
        bool creatable : 1;
#if FEATURE_ESTD_RTTO_GET_INVOKE
        bool invokable : 1;
#endif
    };

    // Use case is where derived class wants to report its RTTO to anyone consuming it.
    // Primary case, designed initially for estd::function to have sensible destroy operations
    class base
    {
        using this_type = base;

    protected:
        utility_type u_;

        /*
         * FIX: Causes a recursion stack overflow
        ~utility_base()
        {
            if(u_)  u_(DELETE, this, nullptr);
        }   */

    public:
        ESTD_CPP_DEFAULT_RULE_OF_5(base)
        constexpr explicit base(utility_type u) :
            u_{u}
        {}

        // EXPERIMENTAL
        int create(int sz = 0)
        {
            return u_(CREATE, this, sz, nullptr);
        }

        constexpr int copy_to(void* dest, int sz = 0) const
        {
            return internal::copy_to(u_, const_cast<this_type*>(this), dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to(void* dest, int sz = 0)
        {
            return internal::move_to(u_, this, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to_and_destroy(void* dest, int sz = 0)
        {
            return internal::move_to_and_destroy(u_, this, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) void destroy()
        {
            internal::destroy(u_, this);
        }

        constexpr int size() const
        {
            return internal::size(u_);
        }

        int get_metadata(const metadata** out) const
        {
            return u_(GET_METADATA, nullptr, 0, out);
        }
    };


    // Case where someone who is NOT target type wants to help with target operations
    // Think of this as the 'has a' vs implied 'is a' of base
    template <class Storage = char[]>
    class proxy : public base
    {
        using base_type = base;

    protected:
        Storage storage_;

    public:
        explicit constexpr proxy(utility_type u) :
            base_type(u)
        {}

        // DEBT: Overlap with above?
        template <class ...Args>
        explicit constexpr proxy(utility_type u, Args&&...args) :
            base_type(u),
            storage_{std::forward<Args>(args)...}
        {}

        // EXPERIMENTAL, kind of a substitute for emplace
        // Won't work with exotic storage types
        template <class T, class ...Args>
        explicit proxy(in_place_type_t<T>, Args&&...args) :
            base_type(rtto<T>::utility)
        {
            // DEBT: I bet there's a way to get this constexpr-friendly
            new (storage_) T(std::forward<Args>(args)...);
        }

        // NOTE: Can default move constructor have an additional parameter like this? Hmm... probably?
        explicit proxy(proxy&& move_from, int* rc, int sz = 0) :
            base_type(move_from.u_)
        {
            // Leaving u_ intact in move_from. A destroy on a moved object is 100% correct and only
            // possible if we leave that u_ there
            *rc = move_from.move_to(storage_, sz);
        }

        explicit proxy(const proxy& copy_from, int* rc, int sz = 0) :
            base_type(copy_from.u_)
        {
            *rc = copy_from.copy_to(storage_, sz);
        }

        explicit proxy(proxy&& move_from) : base_type(move_from.u_)
        {
            assert(move_from.move_to(storage_) == 0);
        }

        explicit proxy(const proxy& copy_from) : base_type(copy_from.u_)
        {
            assert(copy_from.move_to(storage_) == 0);
        }

        // EXPERIMENTAL
        int create(int sz = 0)
        {
            return u_(CREATE, storage_, sz, nullptr);
        }

        constexpr int copy_to(void* dest, int sz = 0) const
        {
            return internal::copy_to(u_, storage_, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to(void* dest, int sz = 0)
        {
            return internal::move_to(u_, storage_, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) int move_to_and_destroy(void* dest, int sz = 0)
        {
            return internal::move_to_and_destroy(u_, storage_, dest, sz);
        }

        ESTD_CPP_CONSTEXPR(14) void destroy()
        {
            internal::destroy(u_, storage_);
        }

        template <class T = void*>
        ESTD_CPP_CONSTEXPR(14) T storage() { return reinterpret_cast<T>(storage_); }
    };

    class virtual_base
    {
    public:
        virtual ~virtual_base() = default;

        virtual int copy_to(void*, int = 0) = 0;
        virtual int move_to(void*, int = 0) = 0;

        void destroy() { this->~virtual_base(); }
    };

    // UNFINISHED, EXPERIMENTAL
    // Seems a fully polymorphic approach *may* be superior to the function ptr dispatch
    // method.  Sussing that out
    class virt
    {
        virtual int copy_to(void* src, void* dst) const = 0;
        virtual int move_to(void* src, void* dst) const = 0;
        virtual void destroy(void*) const = 0;
        virtual int size() const = 0;
    };
};


// TODO: Look into https://en.cppreference.com/w/cpp/memory/polymorphic_allocator.html to
// see if there's any overlap.  So far it doesn't seem so
template <class T, class Traits>
struct rtto :
    rtto_base,
    Traits
{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using traits = Traits;
    using typename traits::is_copy_constructible;
    using typename traits::is_move_constructible;
    using typename traits::is_trivially_constructible;
    using typename traits::is_constructible;
    using typename traits::is_swappable;

    static constexpr int value_sz = sizeof(value_type);

    // EXPERIMENTAL
    //static constexpr metadata mdata{value_sz};

    static ESTD_CPP_CONSTEXPR(14) int copy(const_pointer from, void* to, true_type)
    {
        new (to) value_type(*from);
        return 0;
    }

    static ESTD_CPP_CONSTEXPR(14) int move(pointer from, void* to, true_type)
    {
        new (to) value_type(std::move(*from));
        return 0;
    }

    static ESTD_CPP_CONSTEXPR(14) int create(void* storage, true_type)
    {
        new (storage) value_type;
        return 0;
    }

    // EXPERIMENTAL
    // I'm told this is a well-established paradigm to use instead of a placement-new-for-copy-assignment
    // which helps overcome:
    // - overlapping/to==this conditions
    // - lifecycle concerns (TBD can't remember)
    // - virtual method maint (TBD can't remember)
    static ESTD_CPP_CONSTEXPR(14) int copy_and_swap(const_pointer from, void* to, true_type)
    {
        // DEBT: Make sure this sucker isn't too big for comfort
        // DEBT: Account for alignment/padding
        value_type temp(*from);

        swap(temp, *(pointer)to);
        return 0;
    }

    static constexpr int copy(void*, void*, false_type) { return EINVAL; }
    static constexpr int move(void*, void*, false_type) { return EINVAL; }
    static constexpr int create(void*, false_type) { return EINVAL; }
    static constexpr int copy_and_swap(pointer from, void* to, false_type) { return EINVAL; }

    constexpr static bool size_ok(int sz)
    {
        return sz == 0 || sz >= value_sz;
    }

    static inline int copy(const_pointer from, void *to, int sz)
    {
        if(!size_ok(sz)) return ENOMEM;

        return copy(from, to, is_copy_constructible{});
    }

    static inline int move(pointer from, void *to, int sz)
    {
        if(!size_ok(sz)) return ENOMEM;

        return move(from, to, is_move_constructible{});
    }

    static int utility(modes mode, void* p0, int p1, void* p2)
    {
        auto this_ = static_cast<pointer>(p0);

        switch(mode)
        {
            case COPY:
                // FIX: estd flavor isn't resolving down to estd::true_type/estd::false_type
                //return copy(this_, p2, estd::is_copy_constructible<value_type>{});
                return copy(this_, p2, p1);

            case MOVE:
                return move(this_, p2, p1);

            case MOVE_AND_DESTROY:
                if(int rc = move(this_, p2, p1) != 0) return rc;
                ESTD_CPP_ATTR_FALLTHROUGH;

            case DELETE:
                this_->~value_type();
                break;

            case SIZE:
                return sizeof(value_type);

#if !defined(__AVR__)
            // EXPERIMENTAL
            case CREATE:
                // FIX: estd is_trivially_constructible falls apart here
                return create(p0, is_constructible{});
                //return create(p0, bool_constant<false>{});
#endif

#if FEATURE_ESTD_RTTO_GET_METADATA
            // EXPERIMENTAL
            case GET_METADATA:
            {
                // DEBT: Incorrect convention because Qt Creator loses it's f***in mind here
                static constexpr const metadata mdata{
                    value_sz, is_copy_constructible::value, is_move_constructible::value
                };
                auto dest = static_cast<const metadata**>(p2);
                *dest = &mdata;
                break;
            }
#endif

            // EXPERIMENTAL
            case COPY_AND_SWAP:
                return copy_and_swap(this_, p2, is_swappable{});

            default:
                return ENOSYS;
        }

        return 0;
    }

    // UNFINISHED, EXPERIMENTAL
    // Seems a fully polymorphic approach *may* be superior to the function ptr dispatch
    // method.  Sussing that out
    // While convenient, we do sling the 'this' pointer around for only indirect gains.
    // Possibly an inefficiency as compared to fnptr
    class virt : public rtto_base::virt
    {
    public:
        int copy_to(void* src, void* dest) const override
        {
            return copy(src, dest, is_constructible{});
        }

        int move_to(void* src, void* dest) const override
        {
            return move(src, dest, is_constructible{});
        }

        void destroy(void*) const override
        {
            // TBD
        }

        int size() const override { return value_sz; }
    };
};


}}
