#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

class TimeOut : public std::exception {
    const char* what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !data_.has_value(); });
        data_ = data;
        cv.notify_all();
        cv.wait(lock);
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
    std::unique_lock<std::mutex> lock(mutex);
    if (timeout == std::chrono::milliseconds(0)) {
        cv.wait(lock, [this] { return data_.has_value(); });
    } else {
        if (!cv.wait_for(lock, timeout, [this] { return data_.has_value(); })) {
            throw TimeOut();
        }
    }
    T result = data_.value();
    data_.reset();
    cv.notify_all();
    return result;
}

private:
    std::mutex mutex;
    std::condition_variable cv;
    std::optional<T> data_;
};
