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

template <flags1 f, class Enabled = void>
struct test;

//template <>
//struct test<flags1::opt1> {};

template <flags1 f>
struct test<f, enable_if_t<(f & flags1::opt1).value()>>
{

};

template <flags1 f>
struct test<f, enable_if_t<f & flags1::opt2>>
{

};

template <flags1 f>
struct test<f, enable_if_t<f & (flags1::opt2 | flags1::opt3)>>
{

};


void setup()
{
    Serial.begin(115200);
    while(!Serial);
}

void loop()
{
    test<flags1::opt1> t1;
}
