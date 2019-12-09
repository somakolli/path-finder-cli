cmake_minimum_required(VERSION 3.14)
project(master_arbeit)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/path_finder)

add_executable(master_arbeit main.cpp)

target_link_libraries(master_arbeit path_finder)

set(CMAKE_CXX_FLAGS "-pthread")