#pragma once

#include <thread>
#include <atomic>

class spinwait {
public:
    spinwait() :
        flag(false) {
    }

    void wait() {
        while (flag)
            std::this_thread::yield();
    }
    void lock() {
        flag = true;
    }
    void unlock() {
        flag = false;
    }

private:
    std::atomic<bool> flag;
};