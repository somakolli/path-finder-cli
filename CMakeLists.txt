cmake_minimum_required(VERSION 3.13.4)
project(master_arbeit)

set(CMAKE_CXX_STANDARD 20)

if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(vendor/path_finder)
add_subdirectory(vendor/oscar-routing)

add_executable(file_creator FileCreator.cpp)
add_executable(run_from_files RunFromFiles.cpp)

target_link_libraries(file_creator path_finder oscar-routing)
target_link_libraries(run_from_files path_finder oscar-routing)
set(CMAKE_CXX_FLAGS "-pthread")