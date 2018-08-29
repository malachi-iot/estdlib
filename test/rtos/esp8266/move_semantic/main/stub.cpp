#include <estd/queue.h>

struct NonCopyable
{
    NonCopyable() {}

    //explicit NonCopyable(const NonCopyable& copy_from) {}

    NonCopyable(NonCopyable&& move_from) {}
};

extern "C" void test_task(void*)
{
    estd::layer1::queue<NonCopyable, 5> q;
    estd::layer1::queue<NonCopyable, 5> q2(std::move(q));
}
