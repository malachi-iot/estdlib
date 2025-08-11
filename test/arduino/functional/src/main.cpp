#include <estd/functional.h>
#include <estd/ostream.h>
#include <estd/type_traits.h>

using namespace estd;

static arduino_ostream cout(Serial);


using fn_type = detail::function<unsigned()>;

fn_type fn1;

void setup()
{
    Serial.begin(115200);
    while(!Serial);

    static auto model = fn_type::make_model([]
    {
        static unsigned counter = 0;

        counter += 10;
        return counter;
    });

    fn1 = &model;
}

void loop()
{
    cout << F("Hello World: ") << fn1() << endl;
    delay(1000);
}
