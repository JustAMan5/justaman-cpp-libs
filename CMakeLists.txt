cmake_minimum_required(VERSION 3.20)
project(
    justaman_libs
    VERSION 1.0.0
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config"
)

include(global_options)
include(compiler_flags)
include(dependencies)

# if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     add_compile_options(-ftime-trace)
# elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#     add_compile_options(-ftime-report)
# else()
#     message(WARNING "Active compiler doesnt support time profiling")
# endif()

add_subdirectory(libs) # load the LIBRARIES

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests) # load the tests if project is top level
endif()

if(PROJECT_IS_TOP_LEVEL) # copy the compile commands to the source dir for clangd to find it
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json" 
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()

message(STATUS "------------------------ JustAMan's C++ libs --------------------------")
message(STATUS "Using project version:  ${PROJECT_VERSION}")
message(STATUS "Using CMake:            ${CMAKE_VERSION}")
message(STATUS "C++ compiler:           ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version:   ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Building project in:    ${CMAKE_BUILD_TYPE}")
message(STATUS "Connected to internet:  ${JUSTAMAN_HAS_INTERNET}")
message(STATUS "    * Extra logging:            ${JUSTAMAN_EXTRA_LOGGING}")
message(STATUS "    * Time profiling:           ${JUSTAMAN_TIME_PROFILING}")
message(STATUS "    * Google benchmark tool:    ${JUSTAMAN_FETCH_GOOGLE_BENCHMARK_LIB}")
message(STATUS "    * Warnings as errors:       ${JUSTAMAN_WARNINGS_AS_ERRORS}")
message(STATUS "    * Extra warnings:           ${JUSTAMAN_EXTRA_WARNINGS}")
message(STATUS "    * Hardware optimizations:   ${JUSTAMAN_HARDWARE_OPTIMIZATIONS}")
message(STATUS "    * LTO:                      ${JUSTAMAN_LINK_TIME_OPT}")
message(STATUS "    * Disable exceptions:       ${JUSTAMAN_DISABLE_EXCEPTIONS}")
message(STATUS "    * Disable RTTI:             ${JUSTAMAN_DISABLE_RTTI}")
message(STATUS "    * Fast Math:                ${JUSTAMAN_ENABLE_FAST_MATH}")
message(STATUS "    * Optimize for size:        ${JUSTAMAN_OPTIMIZE_FOR_SIZE}")
message(STATUS "-----------------------------------------------------------------------")