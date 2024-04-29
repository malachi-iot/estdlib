#include "main.h"

// EXPERIMENTAL
// AVR PGM container tests

#if EXP_PGM_STRING
#include <estd/exp/pgm/array.h>

// DEBT: I think we may be able to do some CTAD to deduce array count
static const estd::v0::test_container<int, 4> array1 PROGMEM { 0, 1, 2, 3 };
static constexpr estd::v0::pgm_array<int16_t, 4> array2 PROGMEM { 4, 5, 6, 7 };

namespace avr {

void test_array()
{
    uint32_t dummy = array1[0];
    cout << "pgm_array: " << dummy << ", ";
    
    for(auto dummy2 : array1)
    {
        cout << dummy2 << ", ";
    }

    for(auto dummy2 : array2)
        cout << dummy2 << ", ";

    cout << estd::endl;
}

}

#endif