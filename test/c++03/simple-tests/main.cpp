#include <iostream>

#include <estd/string.h>
#include <estd/ratio.h>

using namespace std;

typedef estd::ratio<1, 100> ratio1;
typedef estd::ratio<1, 30> ratio2;

typedef estd::ratio_divide<ratio1, ratio2> divided;

int main()
{
    estd::layer1::string<20> test_str = "hi2u";

    cout << "Hello World: " << test_str << endl;
    cout << "Ratio: " << divided::num << '/' << divided::den << endl;
    return 0;
}
