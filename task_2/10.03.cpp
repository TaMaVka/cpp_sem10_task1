// g++ -std=c++20 -Wall -Wextra -Wpedantic 10.03.cpp -o 10.03.out
// Note: Boost libraries must be installed for this code to compile.

#include <iostream>
#include <vector>
#include <cstddef> // for size_t
#include <cassert> // for assert
#include <chrono>  // for std::chrono
#include <thread>  // for std::this_thread

#include <boost/multi_array.hpp>

// Constants and Type Definitions
// Grid dimensions for the main simulation.

const size_t GRID_HEIGHT = 10;
const size_t GRID_WIDTH = 10;

// Characters to represent cell states.
const char ALIVE_CELL = '*';
const char DEAD_CELL = '.';

// Simulation parameters.
const int SIMULATION_ITERATIONS = 20;
const std::chrono::milliseconds FRAME_DELAY(200);

// Type alias for a 2D grid using Boost.MultiArray.
using Grid = boost::multi_array<char, 2>;

// Core Game Logic
// Counts the number of live neighbors for a cell at a given position.
// It correctly handles cells on the edges and in the corners of the grid.

int count_live_neighbors(const Grid& grid, size_t row, size_t col)
{
    int live_neighbors = 0;
    const long grid_height_long = static_cast<long>(grid.shape()[0]);
    const long grid_width_long = static_cast<long>(grid.shape()[1]);

    // Iterate through the 3x3 square centered on the cell.
    for (long r_offset = -1; r_offset <= 1; ++r_offset)
    {
        for (long c_offset = -1; c_offset <= 1; ++c_offset)
        {
            // Skip the cell itself.
            if (r_offset == 0 && c_offset == 0)
            {
                continue;
            }

            // Calculate neighbor coordinates, using long to prevent underflow with size_t.
            long neighbor_row = static_cast<long>(row) + r_offset;
            long neighbor_col = static_cast<long>(col) + c_offset;

            // Check if the neighbor is within the grid boundaries.
            if (neighbor_row >= 0 && neighbor_row < grid_height_long &&
                neighbor_col >= 0 && neighbor_col < grid_width_long)
            {
                if (grid[neighbor_row][neighbor_col] == ALIVE_CELL)
                {
                    live_neighbors++;
                }
            }
        }
    }
    return live_neighbors;
}

// Computes the state of the grid for the next generation based on the current state.

Grid compute_next_generation(const Grid& current_grid)
{
    const size_t height = current_grid.shape()[0];
    const size_t width = current_grid.shape()[1];
    Grid next_grid(boost::extents[height][width]);

    for (size_t r = 0; r < height; ++r)
    {
        for (size_t c = 0; c < width; ++c)
        {
            int live_neighbors = count_live_neighbors(current_grid, r, c);
            char current_cell_state = current_grid[r][c];
            char next_cell_state = DEAD_CELL;

            // Rule 1 & 3: A live cell with 2 or 3 neighbors survives.
            if (current_cell_state == ALIVE_CELL && (live_neighbors == 2 || live_neighbors == 3))
            {
                next_cell_state = ALIVE_CELL;
            }
            // Rule 4: A dead cell with exactly 3 neighbors becomes alive.
            else if (current_cell_state == DEAD_CELL && live_neighbors == 3)
            {
                next_cell_state = ALIVE_CELL;
            }
            // All other cells die or remain dead (underpopulation/overpopulation).

            next_grid[r][c] = next_cell_state;
        }
    }
    return next_grid;
}


// Utility Functions
// Clears the console screen for animation effect.

void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    // ANSI escape code for clearing the screen, more portable than system("clear").
    std::cout << "\033[2J\033[1;1H";
#endif
}

// Prints the grid to the standard output.
void print_grid(const Grid& grid, int generation)
{
    clear_screen();
    std::cout << "Conway's Game of Life - Generation: " << generation << "\n";
    const size_t height = grid.shape()[0];
    const size_t width = grid.shape()[1];
    for (size_t r = 0; r < height; ++r)
    {
        for (size_t c = 0; c < width; ++c)
        {
            std::cout << grid[r][c] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << std::flush;
}

// Tests

void test_neighbor_counting()
{
    // A 5x5 test grid to check neighbor counting logic.
    Grid test_grid(boost::extents[5][5]);
    std::fill_n(test_grid.data(), test_grid.num_elements(), DEAD_CELL);

    // Pattern:
    // . * . . .
    // . * . . .
    // . * . * .
    // . . . . .
    // . . . . .
    test_grid[0][1] = ALIVE_CELL;
    test_grid[1][1] = ALIVE_CELL;
    test_grid[2][1] = ALIVE_CELL;
    test_grid[2][3] = ALIVE_CELL;

    // Test 1: Cell in the middle with 3 neighbors.
    assert(count_live_neighbors(test_grid, 1, 0) == 3);

    // Test 2: Live cell with 1 neighbor.
    assert(count_live_neighbors(test_grid, 0, 1) == 1);

    // Test 3: Corner cell with 2 neighbors.
    assert(count_live_neighbors(test_grid, 0, 0) == 2);

    // Test 4: Edge cell with 2 neighbors.
    assert(count_live_neighbors(test_grid, 2, 0) == 2);

    // Test 5: Cell with 0 neighbors.
    assert(count_live_neighbors(test_grid, 4, 4) == 0);

    // Test 6: Cell with neighbors on multiple sides.
    assert(count_live_neighbors(test_grid, 2, 2) == 3);
}

void test_state_transitions()
{
    Grid current_grid(boost::extents[3][3]);
    std::fill_n(current_grid.data(), current_grid.num_elements(), DEAD_CELL);

    // Blinker pattern (vertical bar)
    // . * .
    // . * .
    // . * .
    current_grid[0][1] = ALIVE_CELL;
    current_grid[1][1] = ALIVE_CELL;
    current_grid[2][1] = ALIVE_CELL;

    Grid next_grid = compute_next_generation(current_grid);

    // Test 1: Center cell should survive (2 neighbors).
    assert(next_grid[1][1] == ALIVE_CELL);

    // Test 2: Top cell should die (1 neighbor - underpopulation).
    assert(next_grid[0][1] == DEAD_CELL);

    // Test 3: Side cells should become alive (3 neighbors - reproduction).
    assert(next_grid[1][0] == ALIVE_CELL);
    assert(next_grid[1][2] == ALIVE_CELL);

    // Test 4: Corner cell should remain dead (2 neighbors).
    assert(next_grid[0][0] == DEAD_CELL);
}

// Demonstration

void run_game_of_life_demonstration()
{
    Grid grid(boost::extents[GRID_HEIGHT][GRID_WIDTH]);
    std::fill_n(grid.data(), grid.num_elements(), DEAD_CELL);

    // Initialize with a "Glider" pattern.
    // . * . . .
    // . . * . .
    // * * * . .
    // . . . . .
    grid[0][1] = ALIVE_CELL;
    grid[1][2] = ALIVE_CELL;
    grid[2][0] = ALIVE_CELL;
    grid[2][1] = ALIVE_CELL;
    grid[2][2] = ALIVE_CELL;

    for (int i = 0; i <= SIMULATION_ITERATIONS; ++i)
    {
        print_grid(grid, i);
        std::this_thread::sleep_for(FRAME_DELAY);
        grid = compute_next_generation(grid);
    }
}

// Main Entry Point

void run_tests_and_demonstration()
{
    // Run unit tests first to ensure correctness.
    test_neighbor_counting();
    test_state_transitions();
    std::cout << "Unit tests passed successfully.\n";
    std::cout << "Starting Game of Life demonstration...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Run the main simulation.
    run_game_of_life_demonstration();

    std::cout << "\nAll tests and demonstrations passed successfully.\n";
}

int main()
{
    try
    {
        run_tests_and_demonstration();
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}