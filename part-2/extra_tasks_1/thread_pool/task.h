#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

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