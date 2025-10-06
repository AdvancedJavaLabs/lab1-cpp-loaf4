#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "Graph.h"
#include "RandomGraphGenerator.h"

static long long executeSerialBfsAndGetTime(Graph& g) {
    auto start = std::chrono::steady_clock::now();
    g.bfs(0);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

static long long executeParallelBfsAndGetTime(Graph& g, int numThreads = 4) {
    auto start = std::chrono::steady_clock::now();
    g.parallelBFS(0, numThreads);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
    try {
        std::vector<int> sizes       = {10, 100, 1000, 10000, 10000, 50000, 100000, 1000000, 2000000, 20000000};
        std::vector<int> connections = {50, 500, 5000, 50000, 100000, 1000000, 1000000, 10000000, 10000000, 50000000};

        std::mt19937_64 r(42);

        std::filesystem::create_directories("tmp");
        std::ofstream fw("tmp/results.txt");
        if (!fw) {
            std::cerr << "Failed to open tmp/results.txt for writing\n";
            return 1;
        }

        RandomGraphGenerator gen;
        for (size_t i = 0; i < sizes.size(); ++i) {
            std::cout << "--------------------------\n";
            std::cout << "Generating graph of size " << sizes[i] << " ... wait\n";
            Graph g = gen.generateGraph(r, sizes[i], connections[i]);
            std::cout << "Generation completed!\nStarting bfs\n";
            long long serialTime = executeSerialBfsAndGetTime(g);
            long long parallelTime = executeParallelBfsAndGetTime(g);

            fw << "Times for " << sizes[i] << " vertices and " << connections[i] << " connections: ";
            fw << "\nSerial: " << serialTime;
            fw << "\nParallel: " << parallelTime;
            fw << "\n--------\n";
            fw.flush();
        }
        std::cout << "Done. Results in tmp/results.txt\n";

        std::ofstream fw_graph("tmp/results_graph.txt");
        if (!fw_graph) {
            std::cerr << "Failed to open tmp/results_graph.txt for writing\n";
            return 1;
        }

        std::cout << "Start serial BFS\n";
        for (size_t i = 0; i < sizes.size(); ++i) {
            std::cout << "Generating graph of size " << sizes[i] << " ... wait\n";
            Graph g = gen.generateGraph(r, sizes[i], connections[i]);
            long long serialTime = executeSerialBfsAndGetTime(g);
            fw_graph << serialTime << " ";
        }
        fw_graph << std::endl;
        for (int j = 2; j <= 32; j *= 2) {
            std::cout << "Start parallel BFS with " << j << " cores\n";
            for (size_t i = 0; i < sizes.size(); ++i) {
                std::cout << "Generating graph of size " << sizes[i] << " ... wait\n";
                Graph g = gen.generateGraph(r, sizes[i], connections[i]);
                long long parallelTime = executeParallelBfsAndGetTime(g, j);
                fw_graph << parallelTime << " ";
            }
            fw_graph << std::endl;
        }

        std::cout << "Done. Results in tmp/results.txt\n";
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}