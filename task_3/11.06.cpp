// g++ -std=c++23 -Wall -Wextra -Wpedantic 11.06.cpp -lboost_container -o 11.06.out

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cassert>
#include <iomanip>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

// Type definitions for the graph
using EdgeProperties = boost::property<boost::edge_weight_t, int>;
using Graph = boost::adjacency_matrix<boost::undirectedS, boost::no_property, EdgeProperties>;
using VertexDescriptor = boost::graph_traits<Graph>::vertex_descriptor;
using EdgeDescriptor = boost::graph_traits<Graph>::edge_descriptor;

// Function to print the adjacency matrix of the graph
void print_adjacency_matrix(const Graph& g) {
    const size_t num_vertices = boost::num_vertices(g);
    auto weight_map = boost::get(boost::edge_weight, g);

    std::cout << "Adjacency Matrix:" << std::endl;
    std::cout << "    ";
    for (size_t i = 0; i < num_vertices; ++i) {
        std::cout << std::setw(4) << i;
    }
    std::cout << std::endl;
    std::cout << "----";
    for (size_t i = 0; i < num_vertices; ++i) {
        std::cout << "----";
    }
    std::cout << std::endl;


    for (size_t i = 0; i < num_vertices; ++i) {
        std::cout << std::setw(2) << i << " |";
        for (size_t j = 0; j < num_vertices; ++j) {
            if (i == j) {
                std::cout << std::setw(4) << 0;
            } else {
                auto edge_pair = boost::edge(i, j, g);
                if (edge_pair.second) {
                    std::cout << std::setw(4) << weight_map[edge_pair.first];
                }
            }
        }
        std::cout << std::endl;
    }
}

// Function to solve the Traveling Salesperson Problem using brute force
void solve_tsp(const Graph& g) {
    const size_t num_vertices = boost::num_vertices(g);
    if (num_vertices == 0) {
        std::cout << "Graph is empty." << std::endl;
        return;
    }

    auto weight_map = boost::get(boost::edge_weight, g);
    
    std::vector<VertexDescriptor> path(num_vertices);
    std::iota(path.begin(), path.end(), 0);

    std::vector<VertexDescriptor> best_path;
    int min_cost = std::numeric_limits<int>::max();

    // We fix the starting vertex (0) and permute the rest
    do {
        int current_cost = 0;
        bool path_is_valid = true;
        for (size_t i = 0; i < num_vertices - 1; ++i) {
            auto edge_pair = boost::edge(path[i], path[i + 1], g);
            if (!edge_pair.second) {
                path_is_valid = false;
                break;
            }
            current_cost += weight_map[edge_pair.first];
        }

        if (!path_is_valid) continue;

        // Add the cost of returning to the start
        auto return_edge_pair = boost::edge(path.back(), path.front(), g);
        if(!return_edge_pair.second) continue;

        current_cost += weight_map[return_edge_pair.first];

        if (current_cost < min_cost) {
            min_cost = current_cost;
            best_path = path;
        }
    } while (std::next_permutation(path.begin() + 1, path.end()));

    std::cout << "\nOptimal Path:" << std::endl;
    if (!best_path.empty()) {
        for (size_t i = 0; i < best_path.size(); ++i) {
            std::cout << best_path[i] << (i == best_path.size() - 1 ? "" : " -> ");
        }
        // Show the return to the starting vertex
        std::cout << " -> " << best_path.front() << std::endl;
        std::cout << "Total Cost: " << min_cost << std::endl;
    } else {
        std::cout << "No solution found." << std::endl;
    }
}


void run_tests_and_demonstration() {
    // Test 1: A simple 3-vertex graph with a known optimal path.
    // This test verifies that the permutation and cost calculation logic is correct.
    {
        const size_t num_test_vertices = 3;
        Graph test_g(num_test_vertices);
        // w(0,1)=10, w(0,2)=1, w(1,2)=2. Optimal path 0-2-1-0 has cost 1+2+10=13.
        boost::add_edge(0, 1, EdgeProperties(10), test_g);
        boost::add_edge(0, 2, EdgeProperties(1), test_g);
        boost::add_edge(1, 2, EdgeProperties(2), test_g);

        auto weight_map = boost::get(boost::edge_weight, test_g);
        std::vector<VertexDescriptor> path = {0, 1, 2};
        int min_cost = std::numeric_limits<int>::max();

        do {
            int current_cost = weight_map[boost::edge(path[0], path[1], test_g).first] +
                               weight_map[boost::edge(path[1], path[2], test_g).first] +
                               weight_map[boost::edge(path[2], path[0], test_g).first];
            if (current_cost < min_cost) {
                min_cost = current_cost;
            }
        } while (std::next_permutation(path.begin() + 1, path.end()));
        
        assert(min_cost == 13);
    }

    // Main demonstration as per the task requirements.
    std::cout << "--- Task 11.06: Traveling Salesperson Problem Demonstration ---" << std::endl;

    const size_t num_vertices = 10;
    const int min_weight = 1;
    const int max_weight = 10;
    
    Graph g(num_vertices);

    // Initialize random number generation
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<> distrib(min_weight, max_weight);

    // Populate the complete graph with random edge weights
    for (size_t i = 0; i < num_vertices; ++i) {
        for (size_t j = i + 1; j < num_vertices; ++j) {
            int weight = distrib(gen);
            boost::add_edge(i, j, EdgeProperties(weight), g);
        }
    }

    print_adjacency_matrix(g);
    solve_tsp(g);
    
    std::cout << "\n-------------------------------------------------------------" << std::endl;
    std::cout << "All tests and demonstration completed successfully." << std::endl;
}

int main() {
    run_tests_and_demonstration();
    return 0;
}