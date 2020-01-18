// Two peculiar things:
// 1. unsigned long for blackfin is 32-bit
// 2. existing 32-bit numeric limits resolver doesn't appear to connect to #1

#include <string>
#include <estd/string.h>
#include <estd/limits.h>

// Works but doesn't have std::numeric_limits in it
#include <climits>


void do_string_stuff()
{
	typedef estd::char_traits<char> traits_type;
	estd::layer1::string<100> str;
	//int max = estd::numeric_limits<int64_t>::max();
	unsigned long max = estd::numeric_limits<unsigned long>::max();
	
	// has trouble with numeric_limits somehow size_t not resolving
	str += "Hello";
	estd::internal::deduce_fixed_size_t<128>::size_type val;
}