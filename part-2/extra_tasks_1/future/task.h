#pragma once
#include "ThreadPool.h"

#include <algorithm>
#include <vector>
#include <deque>
#include <cstdint>
#include <atomic>
#include <memory>
#include <future>
// нельзя инклюдить set и queue

/*
    Задание -  реализовать многопоточный алгоритм Дейкстры (подробнее: https://neerc.ifmo.ru/wiki/index.php?title=Алгоритм_Дейкстры).
    Вам надо написать также ThreadPool, который необходимо использовать для распараллеливания вычислений.
    Для этого требуется реализовать функци findMinVertex и relaxEdges.

    `findMinVertex` - Находит вершину с минимальным расстоянием, которая ещё не была посещена и возвращает ее.

    `relaxEdges` - Производит релаксацию ребер из полученной в `findMin` вершины по всем исходящим вершинам

    Обе функции должны работать с графом в ThreadPool.

    Чтобы проверить, исполнена задача или нет
    if(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    получение и обработка результата
    }

    Гарантируется что не случится переполнение int.
*/

// Важно реализовать алгоритм с асимптотикой O(n^2)

class Dijkstra
{
public:
    Dijkstra(int numVertices, int numEdges, int numThreads) : threadPool(numThreads), threadCount(numThreads), numVertices(numVertices),
     distances(numVertices, INT_MAX), visited(numVertices, false), graph(numVertices), previous(numVertices, -1) {}

    ~Dijkstra() {
        threadPool.Terminate(true);
    }
    // Добавляет ребра между двумя вершинами с указанным весом в неориентированном графе.
    void addEdge(int from, int to, int weight) {
        from --;
        to --;
        graph[from].emplace_back(to, weight);
        graph[to].emplace_back(from, weight);
    }

    // Выполняет сам алгоритм Дейкстры многопоточно.
    void run(int start, int finish)
    {
        start--;
        finish--;
        distances[start] = 0;
        for (int i = 0; i < numVertices; ++i) {
            int v = findMinVertex();
            if (v == -1 || distances[v] == INT_MAX) {
                break;
            }
            visited[v] = true;
            if (v == finish) {
                break;
            }
            relaxEdges(v);
        }
    }

    // Получает кратчайший путь  в виде последовательности вершин и возвращает его. (вершины выводятся по возврастанию)
    // Если не существует пути, требуется вернуть пустую deque
    std::deque<int> getPath(int start, int finish) {
        start--;
        finish--;
        std::deque<int> path;
        for (int v = finish; v != -1 && previous[v] != -1; v = previous[v]) {
            path.push_front(v + 1);
        }
        if (path.empty()) {
            return {};
        }
        path.push_front(start + 1);
        return path;
    }

private:
    ThreadPool threadPool;
    int threadCount;
    int numVertices;
    std::vector<int> distances;
    std::vector<bool> visited;
    std::vector<std::vector<std::pair<int, int>>> graph;
    std::mutex graphMutex;
    std::vector<int> previous;

    // Находит вершину с минимальным расстоянием, которая ещё не была посещена.
    int findMinVertex() {
        int numTasks = threadCount;
        int block_size = (numVertices + numTasks - 1) / numTasks;
        std::vector<std::future<std::pair<int, int>>> futures;
        for (int start = 0; start < numVertices; start += block_size) {
            int end = std::min(start + block_size, numVertices);
            std::promise<std::pair<int, int>> promise;
            futures.push_back(promise.get_future());
            auto prom_ptr = std::make_shared<std::promise<std::pair<int,int>>>(std::move(promise));
            threadPool.PushTask([this, start, end, prom_ptr]() mutable {
                int localMin = INT_MAX;
                int localIndex = -1;
                for (int i = start; i < end; ++i) {
                    if (!visited[i] && distances[i] < localMin) {
                        localMin = distances[i];
                        localIndex = i;
                    }
                }
                prom_ptr->set_value(std::make_pair(localIndex, localMin));
            });

        }
        int minVertex = -1;
        int minDistance = INT_MAX;
        for (auto& future : futures) {
            auto result = future.get();
            if (result.first != -1 && result.second < minDistance) {
                minDistance = result.second;
                minVertex = result.first;
            }
        }
        return minVertex;
    }

    // Производит релаксацию ребер из vertex
    void relaxEdges(int vertex) {
        int currentDistance = distances[vertex];
        const auto &edges = graph[vertex];
        int edgeCount = edges.size();
        int block_size = (edgeCount + threadCount - 1) / threadCount;

        std::vector<std::future<void>> futures;
        for (int start = 0; start < edgeCount; start += block_size) {
            int end = std::min(start + block_size, edgeCount);
            std::promise<void> promise;
            futures.push_back(promise.get_future());
            auto prom_ptr = std::make_shared<std::promise<void>>(std::move(promise));
            threadPool.PushTask([this, vertex, currentDistance, start, end, prom_ptr]() mutable {
                for (int i = start; i < end; ++i) {
                    int u = graph[vertex][i].first;
                    int weight = graph[vertex][i].second;
                    int newDistance = currentDistance + weight;
                    std::lock_guard<std::mutex> lock(graphMutex);
                    if (newDistance < distances[u]) {
                        distances[u] = newDistance;
                        previous[u] = vertex;
                    }
                }
                prom_ptr->set_value();
            });
        }
        for (auto& future : futures) {
            future.get();
        }
    }
};
