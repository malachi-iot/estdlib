#pragma once

namespace estd { namespace test {

struct Dummy
{
    int val1;
    const char* value2;

    // because underlying struct is an array for layer1::queue, darnit
    Dummy() {}

    Dummy(int val1, const char* val2) :
        val1(val1), value2(val2)
        {}

    Dummy(Dummy&& move_from) :
        val1(move_from.val1),
        value2(move_from.value2)
    {

    }

    Dummy(const Dummy& copy_from) :
        val1(copy_from.val1),
        value2(copy_from.value2)
    {

    }

    Dummy& operator =(const Dummy& copy_from)
    {
        new (this) Dummy(copy_from);
        return *this;
    }
};

static uint8_t octet_data[] { 1, 2, 3, 4, 5, 6, 7, 8, 9 };


}}
