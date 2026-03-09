cmake_minimum_required(VERSION 3.10)
project(DynamicLibraryLoader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost (required for Boost.DLL)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Create two shared libraries
add_library(shared1 SHARED lib1.cpp)
add_library(shared2 SHARED lib2.cpp)

# Create executable
add_executable(06.05.out 06.05.cpp)

# Link Boost libraries to the executable
target_link_libraries(06.05.out PRIVATE Boost::system Boost::filesystem)

# Optionally set output names for libraries
set_target_properties(shared1 PROPERTIES OUTPUT_NAME "shared1")
set_target_properties(shared2 PROPERTIES OUTPUT_NAME "shared2")