#include <chrono>
#include <mutex>
#include <thread>

#include <estd/chrono.h>

using namespace std::chrono_literals;

static void chrono_proving()
{
    std::chrono::seconds s(5);

    auto now1 = std::chrono::system_clock::now();
    auto now2 = std::chrono::steady_clock::now();
    auto now3 = std::chrono::high_resolution_clock::now();
}

static void thread_proving()
{
    // Not available as of SDK v1.4.0

    //std::this_thread::sleep_for(100ms);
    //std::mutex m;

    // ---
    // DEBT: This won't compile because the std -> estd duration conversion
    // is not working
    //estd::this_core::sleep_for(100ms);
}


void proving()
{
    chrono_proving();
    thread_proving();
}