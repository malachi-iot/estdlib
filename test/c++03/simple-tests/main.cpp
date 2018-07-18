#include <iostream>

#include <estd/string.h>

using namespace std;

int main()
{
    estd::layer1::string<20> test_str = "hi2u";

    cout << "Hello World: " << test_str << endl;
    return 0;
}
