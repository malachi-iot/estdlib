#include <estd/flags.h>
#include <estd/type_traits.h>

using namespace estd;

enum class flags1
{
    opt0 = 0x00,
    opt1 = 0x01,
    opt2 = 0x02,
    opt3 = 0x04
};

ESTD_FLAGS(flags1)
//ESTD_FLAGS_ENABLE_IF(flags1)

template <flags1 f, class Enabled = void>
struct test;

//template <>
//struct test<flags1::opt1> {};

template <flags1 f>
//struct test<f, enable_if_t<(f & flags1::opt1).operator bool()>>
//struct test<f, enable_if_t<bool(f & flags1::opt1)>>
//struct test<f, if_flagged<f & flags1::opt1>>
//struct test<f, enable_if_t<f & flags1::opt1>>
struct test<f, enable_if_t<is_set(f & flags1::opt1)>>
//struct test<f, typename enable_if<internal::flagged(f & flags1::opt1)>::type>
//struct test<f, typename flags1_enable_if<f & flags1::opt1>::type>
//struct test<f, enable_if_t<true>>
//struct test<f, enable_if_t<(f & flags1::opt1)()>>
{
    static constexpr flags1 flags = f;
};

template <flags1 f>
struct test<f, enable_if_t<f & flags1::opt2>>
{
    static constexpr flags1 flags = f;
};

template <flags1 f>
struct test<f, enable_if_t<f & (flags1::opt2 | flags1::opt3)>>
{
    static constexpr flags1 flags = f;
};


void setup()
{
    Serial.begin(115200);
    while(!Serial);
}

void loop()
{
    test<flags1::opt1> t1;

    static_assert(decltype(t1)::flags == flags1::opt1);
}
