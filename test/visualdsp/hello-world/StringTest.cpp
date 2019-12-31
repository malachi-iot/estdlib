#include <string>
#include <estd/string.h>

void do_string_stuff()
{
	typedef estd::char_traits<char> traits_type;
	// FIX: appears deduced size type is coming out int16_t,
	// expecting int8_t
	// TODO: repair deduce fixed size warnings
	estd::layer1::string<100> str;
	//estd::internal::deduce_fixed_size_t<128>::size_type val;
}