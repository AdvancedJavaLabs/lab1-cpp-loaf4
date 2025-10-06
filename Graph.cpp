#include "Graph.h"
#include <algorithm>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>

Graph::Graph(int vertices) : V(vertices), adjList(vertices) {}

void Graph::addEdge(int src, int dest) {
    if (src < 0 || dest < 0 || src >= V || dest >= V) return;
    auto& vec = adjList[src];
    if (std::find(vec.begin(), vec.end(), dest) == vec.end()) {
        vec.push_back(dest);
    }
}

void Graph::parallelBFS(int startVertex, int numThreads) {
    if (startVertex < 0 || startVertex >= V) return;

    std::vector<std::atomic<bool>> visited(V);
    for (auto& v : visited) {
        v.store(false);
    }

    std::vector<int> currentLevel;
    std::vector<int> nextLevel;
    std::mutex nextLevelMutex;

    visited[startVertex].store(true);
    currentLevel.push_back(startVertex);
    
    int level = 0;
    
    while (!currentLevel.empty()) {
        auto processVertices = [&](int startIdx, int endIdx) {
            std::vector<int> localNextLevel;
            
            for (int i = startIdx; i < endIdx; ++i) {
                int u = currentLevel[i];
                
                for (int n : adjList[u]) {
                    bool expected = false;
                    if (visited[n].compare_exchange_strong(expected, true)) {
                        localNextLevel.push_back(n);
                    }
                }
            }

            if (!localNextLevel.empty()) {
                std::lock_guard<std::mutex> lock(nextLevelMutex);
                nextLevel.insert(nextLevel.end(), localNextLevel.begin(), localNextLevel.end());
            }
        };

        std::vector<std::thread> threads;
        int chunkSize = (currentLevel.size() + numThreads - 1) / numThreads;
        
        for (int i = 0; i < numThreads; ++i) {
            int start = i * chunkSize;
            int end = std::min(start + chunkSize, static_cast<int>(currentLevel.size()));
            
            if (start < currentLevel.size()) {
                threads.emplace_back(processVertices, start, end);
            }
        }
        for (auto& thread : threads) {
            thread.join();
        }

        currentLevel.swap(nextLevel);
        nextLevel.clear();
        ++level;
    }
}

void Graph::bfs(int startVertex) {
    if (startVertex < 0 || startVertex >= V) return;
    std::vector<char> visited(V, 0);
    std::queue<int> q;

    visited[startVertex] = 1;
    q.push(startVertex);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int n : adjList[u]) {
            if (!visited[n]) {
                visited[n] = 1;
                q.push(n);
            }
        }
    }
}

int Graph::vertices() const { return V; }