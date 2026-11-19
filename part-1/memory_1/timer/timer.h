#include <iostream>
#include <chrono>
#include <sstream>

class Timer {
public:
    Timer(std::chrono::seconds time) : start_time(std::chrono::steady_clock::now()), end_time(start_time + time) {}

    bool Expired() const {
        std::chrono::steady_clock::time_point now_time = std::chrono::steady_clock::now();
        if (now_time >= end_time) {
            return true;
        } else {
            return false;
        }
    }
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
};

class TimeMeasurer {
public:
    TimeMeasurer(std::ostream& os) : os_(os), start_(std::chrono::steady_clock::now()) {}
    ~TimeMeasurer(){
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start_).count();
        os_ << "Elapsed time: " << elapsed << " ms\n";
    }

private:
    std::ostream& os_; 
    std::chrono::steady_clock::time_point start_;
};
