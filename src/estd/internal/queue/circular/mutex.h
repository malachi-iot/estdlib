#pragma once

#include "../../platform.h"
#include "enum.h"

#if FEATURE_STD_MUTEX
#include <mutex>
#endif

namespace estd { namespace internal {

// Only for testing
struct circular_mutex_synthetic
{
    int front_ = 0;
    int back_ = 0;
    int count_ = 0;

    void lock_front() { ++front_; }
    static void unlock_front() { }
    void lock_back() { ++back_; }
    static void unlock_back() { }
    void lock_count() { ++count_; }
    static void unlock_count() { }
};

struct circular_mutex_noop
{
    static constexpr bool lock_front() { return true; }
    static constexpr bool unlock_front() { return {}; }
    static constexpr bool lock_back() { return true; }
    static constexpr bool unlock_back() { return {}; }
    static constexpr bool lock_count() { return true; }
    static constexpr bool unlock_count() { return {}; }
};

#if FEATURE_STD_MUTEX
struct circular_mutex_std
{
    std::mutex front_, back_, count_;

    void lock_front() { front_.lock(); }
    void unlock_front() { front_.unlock(); }
    void lock_back() { back_.lock(); }
    void unlock_back() { back_.unlock(); }
    void lock_count() { count_.lock(); }
    void unlock_count() { count_.unlock(); }
};
#endif


}}