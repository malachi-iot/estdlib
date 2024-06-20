#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/locale.h>

#define USE_ALT_POLICY 0

using namespace estd;

template <class Streambuf, template <class, class> class Policy, class Locale = internal::default_locale>
using basic_ostream_with_policy =
    detail::basic_ostream<Streambuf,
        internal::basic_ios<Streambuf, false, Policy<Streambuf, Locale> > >;

#if USE_ALT_POLICY
template <class Streambuf, class Locale>
struct alt_policy : internal::ios_base_policy<Streambuf, Locale>
{
    using cbase_policies = internal::cbase_policies;

    static constexpr cbase_policies cbase_policy =
        cbase_policies(cbase_policies::CBASE_POLICY_CASE_UPPER | cbase_policies::CBASE_POLICY_HEX_ALWAYS);
};

//static detail::basic_ostream<arduino_ostreambuf,
//    internal::basic_ios<arduino_ostreambuf, false, alt_policy<> > > cout(Serial);
static basic_ostream_with_policy<arduino_ostreambuf, alt_policy> cout(Serial);
#else
static arduino_ostream cout(Serial);
#endif

void setup()
{
    Serial.begin(115200);
}

// NOTE: It feels odd that ostream flavor uses more, since even though
// it's fatter, the price (I thought) was already paid.  It makes sense
// optimizer would have easier time with isolated num_put, though
// atmega32u4, USE_WIDTH=0
// USE_NUM_PUT (0) = 4102
// USE_NUM_PUT (1) = 3890
// atmega32u4, USE_WIDTH=1
// USE_NUM_PUT (0) = 4126
// USE_NUM_PUT (1) = 3988
#define USE_NUM_PUT 0
#define USE_WIDTH 1

void loop()
{
    static unsigned counter = 0;

#if USE_NUM_PUT
    ios_base fmt;

    fmt.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);
#if USE_WIDTH
    fmt.width(4);
#endif

    char val[32];
    char* last;

    last = estd::num_put<char, char*>::put(val, fmt, '0', ++counter);
    *last = 0;

    cout << F("Hello World: ") << val << estd::endl;
#else
#if USE_WIDTH
    cout.width(4);
    cout.fill('0');
#endif
    cout.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);
    cout << F("Hello World: ") << ++counter << estd::endl;
#endif

    delay(1000);
}
