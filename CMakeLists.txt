cmake_minimum_required(VERSION 3.13.4)
project(master_arbeit)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/path_finder)
add_subdirectory(vendor/oscar-routing)

add_executable(master_arbeit main.cpp)
add_executable(file_creator FileCreator.cpp)
add_executable(run_from_files RunFromFiles.cpp)
add_executable(benchmark Benchmark.cpp)

target_link_libraries(master_arbeit path_finder)
target_link_libraries(file_creator path_finder oscar-routing)
target_link_libraries(run_from_files path_finder)
target_link_libraries(benchmark path_finder)
set(CMAKE_CXX_FLAGS "-pthread")