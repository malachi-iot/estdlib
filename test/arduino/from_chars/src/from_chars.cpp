#include <Arduino.h>

#include <estd/string.h>
#include <estd/charconv.h>
#include <estd/ostream.h>

void setup()
{
    Serial.begin(115200);
}

static estd::arduino_ostream cout(Serial);

void loop()
{
    static int counter = 0;

    estd::layer2::const_string number = "1234";

    long value = -1;    // To suppress maybe-uninitalized warning

    // TODO: Show content of r errc if not OK
    //estd::from_chars_result r =
    estd::from_chars(number.data(), number.data() + number.size(), value);

    cout << "Hello: ";
    cout << F("Number - counter=") << counter++;
    cout << F(", value=") << value;

    Serial.println(F("!!"));
    
    delay(1000);
}