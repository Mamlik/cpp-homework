#include "task.h"
#include <vector>
#include <stdexcept>


PrimeNumbersSet::PrimeNumbersSet() {
    nanoseconds_under_mutex_.store(0);
    nanoseconds_waiting_mutex_.store(0);
}

bool PrimeNumbersSet::IsPrime(uint64_t number) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    return primes_.find(number) != primes_.end();
}

uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    auto it = primes_.upper_bound(number);
    if (it == primes_.end()) {
            throw std::invalid_argument("ERR");
        }
    return *it;
}


void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to) {
    auto check = [] (uint64_t x) {
            if (x < 2) return false;
            for (uint64_t i = 2; i * i <= x; ++i) {
                if (x % i == 0) return false;
            }
            return true;
        };
    std::vector<uint64_t> primes_in_range;
    for (uint64_t num = from; num < to; ++num) {
        if (check(num)) {
            primes_in_range.push_back(num);
        }
    }


    auto t1 = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(set_mutex_);
    auto t2 = std::chrono::steady_clock::now();
    nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

    auto t3 = std::chrono::steady_clock::now();
    primes_.insert(primes_in_range.begin(), primes_in_range.end());
    auto t4 = std::chrono::steady_clock::now();
    nanoseconds_under_mutex_ += 
        std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
}

size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    auto it1 = primes_.lower_bound(from);
    auto it2 = primes_.upper_bound(to - 1);
    return std::distance(it1, it2);
}

uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    return primes_.empty() ? 0 : *primes_.rbegin();
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const {
    return std::chrono::nanoseconds(nanoseconds_waiting_mutex_);
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const {
    return std::chrono::nanoseconds(nanoseconds_under_mutex_);
}
