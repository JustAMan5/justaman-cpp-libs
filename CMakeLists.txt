cmake_minimum_required(VERSION 3.20)
project(justaman_libs LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libs)
add_subdirectory(tests)

message(STATUS "")
message(STATUS "Using CMake:            ${CMAKE_VERSION}")
message(STATUS "Compiler C++:           ${CMAKE_CXX_COMPILER}")
message(STATUS "Building project in:    ${CMAKE_BUILD_TYPE}")