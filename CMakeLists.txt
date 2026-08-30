cmake_minimum_required(VERSION 3.20)
project(
    justaman_libs
    VERSION 1.0.2 ## later should be version 1.1.0
    LANGUAGES CXX
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(PROJECT_IS_TOP_LEVEL) # copy the compile commands to the source dir
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json" 
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()

add_subdirectory(libs)
if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(tests)
endif()

message(STATUS "------------------------ JustAMan's C++ libs --------------------------")
message(STATUS "Using project version:  ${PROJECT_VERSION}")
message(STATUS "Using CMake:            ${CMAKE_VERSION}")
message(STATUS "Compiler C++:           ${CMAKE_CXX_COMPILER}")
message(STATUS "Building project in:    ${CMAKE_BUILD_TYPE}")
message(STATUS "-----------------------------------------------------------------------")