#pragma once
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>
#include <queue>
#include <thread>
#include <condition_variable>
#include <cstdint>


class ThreadPool {
    public:
        ThreadPool(size_t threadCount) : active(true) {
            for (size_t i = 0; i < threadCount; ++i) {
                threads.emplace_back([this]() {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(tasks_mutex);
                            tasks_cv.wait(lock, [this]() { return !active || !tasks.empty(); });
                            //////отмена или пусто
                            if (tasks.empty() && !active) {
                                break;
                            }
                            //////
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                });
            }
        }
    
        void PushTask(const std::function<void()>& task) {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            if (!active) {
                throw std::runtime_error("terminated");
            }
            tasks.push(task);
            tasks_cv.notify_one();
        }
    
        void Terminate(bool wait) {
            {
                std::unique_lock<std::mutex> lock(tasks_mutex);
                active = false;
                if (!wait) {
                    while (!tasks.empty()) {
                        tasks.pop();
                    }
                }
            }
            tasks_cv.notify_all();
            for (auto& thread : threads) {
                thread.join();
            }
        }
    
        bool IsActive() const {
            return active.load();
        }
    
        size_t QueueSize() const {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            return tasks.size();
        }
    private:
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> tasks;
        mutable std::mutex tasks_mutex;
        std::condition_variable tasks_cv;
        std::atomic<bool> active;
};