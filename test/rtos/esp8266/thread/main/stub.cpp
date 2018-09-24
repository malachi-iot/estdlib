#include <estd/thread.h>
#include <estd/mutex.h>

extern "C" void test_task(void*)
{
    estd::experimental::mutex m;
    estd::unique_lock<estd::experimental::mutex> t(m);
}


extern "C" void got_ip_event() 
{
    
}