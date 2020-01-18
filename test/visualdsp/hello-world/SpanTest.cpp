#include <estd/cstdint.h>
#include <estd/span.h>

void do_span_stuff()
{
	char buf[128];
	estd::span<char> span(buf);
}