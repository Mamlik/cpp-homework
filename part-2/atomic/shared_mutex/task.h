#pragma once
#include <atomic>

class SharedMutex {
public:
    void lock() {
        while (locked_unique.exchange(true, std::memory_order_acquire)) {
            locked_unique.wait(true, std::memory_order_acquire);
        }
        int currentReaders = readers.load(std::memory_order_acquire);
        while (currentReaders != 0) {
            readers.wait(currentReaders, std::memory_order_acquire);
            currentReaders = readers.load(std::memory_order_acquire);
        }
    }

    void unlock() {
        locked_unique.store(false, std::memory_order_release);
        locked_unique.notify_all();
    }

    void lock_shared() {
            while (locked_unique.load(std::memory_order_acquire)) {
                locked_unique.wait(true, std::memory_order_acquire);
            }
            readers.fetch_add(1, std::memory_order_acquire);
    }

    void unlock_shared() {
        int oldReaders = readers.fetch_sub(1, std::memory_order_release);
        if (oldReaders == 1) {
            readers.notify_all();
        }
    }

private:
    std::atomic<bool> locked_unique {false};
    std::atomic<int> readers {0};
};
