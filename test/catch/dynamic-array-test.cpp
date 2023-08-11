#include <catch.hpp>

#include <estd/internal/dynamic_array.h>

using namespace estd;
using namespace estd::internal;

#include "mem.h"

#include "macro/push.h"

struct pa_impl
{
    //ESTD_CPP_STD_VALUE_TYPE(int)

    //int data_[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
};

template <>
struct experimental::private_array<pa_impl> : public array<int, 10>
{
    using base_type = array<int, 10>;

    private_array() : base_type{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
    {

    }

    void copy_ll(int* dest, int count) const
    {
        estd::copy_n(base_type::data(), count, dest);
    }
};

static constexpr const int data_[] = {0, 1, 2, 3};

struct synthetic_impl
{
    using size_type = unsigned;

    using allocator_type = _allocator<const int>;
    struct allocator_traits : estd::allocator_traits<allocator_type>
    {
        using allocator_type = _allocator<const int>;
        static constexpr const internal::allocator_locking_preference::_ locking_preference =
            internal::allocator_locking_preference::iterator;
    };

    static allocator_type get_allocator() { return {}; }

    static unsigned size() { return 4; }
    static const int* offset(unsigned v) { return data_ + v; }
};

TEST_CASE("dynamic array")
{
    using da1_type = internal::dynamic_array<internal::impl::dynamic_array<
        estd::internal::single_fixedbuf_allocator<int, 20>, void> >;
    using pa1_type = experimental::private_array<pa_impl>;

    da1_type da1;
    pa1_type pa1;

    SECTION("basics")
    {
        da1.push_back(5);

        REQUIRE(da1.size() == 1);

        da1.append(pa1);

        REQUIRE(da1.size() == 11);

        REQUIRE(da1[2] == 1);
    }
    SECTION("iterate")
    {
#if FEATURE_ESTD_ALLOCATED_ARRAY_TRADITIONAL
        int _v0 = 0;
        da1_type::accessor a = _v0;
#else
        da1_type::accessor a(da1.get_allocator(), 0);
#endif
        da1.append(pa1);

        auto it = da1.begin();

        const auto& v = *it;

        REQUIRE(v == 0);

        int v2[] = { 10, 20, 30 };

        da1_type::a_it::accessor a2(v2[0]);
        da1_type::a_it it2(a2);

        a2.h_exp() += 1;

        REQUIRE(a2 == 20);
    }
    SECTION("overflow")
    {
#if FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
        // DEBT: Consider adding assignment operator to append_result
        auto r = da1.append(pa1);
        REQUIRE(r.has_value());
        auto r2 = da1.append(pa1);
        REQUIRE(r2.has_value());
        //auto r3 = da1.append(pa1);
        //REQUIRE(r3.has_value() == false);
        //REQUIRE(r3.error() == 0);
#endif
    }
    SECTION("helper")
    {
        int dest[10];
        estd::internal::allocated_array<synthetic_impl> a{};
        using helper = estd::internal::dynamic_array_helper<synthetic_impl>;

        helper::copy_to(a, dest, 2);

        REQUIRE(dest[1] == 1);
    }
    SECTION("assign/copy")
    {
        estd::internal::allocated_array<synthetic_impl> a{};
        da1_type da1_1;

        da1.push_back(1);

        da1_1 = da1;

        REQUIRE(da1_1.size() == 1);
    }
    SECTION("equality")
    {
        estd::internal::allocated_array<synthetic_impl> a{};

        da1.append(a);
    }
    SECTION("insert")
    {
        da1.push_back(1);
        da1.push_back(2);
        da1.push_back(3);

        da1.insert(da1.begin() + 1, 4);

        REQUIRE(da1[0] == 1);
        REQUIRE(da1[1] == 4);
        REQUIRE(da1[2] == 2);
        REQUIRE(da1[3] == 3);

        REQUIRE(da1.size() == 4);
    }
}

#include "macro/pop.h"
