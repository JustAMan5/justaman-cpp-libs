cmake_minimum_required(VERSION 3.20)
project(
    justaman_libs
    VERSION 1.0.0
    LANGUAGES CXX
)


if(PROJECT_IS_TOP_LEVEL) # copy the compile commands to the source dir for clangd to find it
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json" 
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()

# if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     add_compile_options(-ftime-trace)
# elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#     add_compile_options(-ftime-report)
# else()
#     message(WARNING "Active compiler doesnt support time profiling")
# endif()

add_subdirectory(libs) # load the packages
if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests) # load the tests if project is top level
endif()

message(STATUS "------------------------ JustAMan's C++ libs --------------------------")
message(STATUS "Using project version:  ${PROJECT_VERSION}")
message(STATUS "Using CMake:            ${CMAKE_VERSION}")
message(STATUS "C++ compiler:           ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version:   ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Building project in:    ${CMAKE_BUILD_TYPE}")
message(STATUS "-----------------------------------------------------------------------")