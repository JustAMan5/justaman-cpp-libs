#######################################################################################
# ------------------------------ C++20 SPECIFIC BUILDS ----------------------------- #
#######################################################################################
file(GLOB_RECURSE CPP20_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp20/src/*.cpp")

if(NOT CPP20_SRCS)
    add_library(justaman_cpp20 INTERFACE) # mark the library as interface (no binary needed, only forwards properties when linked)
    set(LINK_TYPE INTERFACE)
else()
    add_library(justaman_cpp20 STATIC ${CPP20_SRCS})
    set(LINK_TYPE PUBLIC)
endif()

add_library(justaman::cpp20 ALIAS justaman_cpp20)
target_compile_features(justaman_cpp20 ${LINK_TYPE} cxx_std_20)
target_include_directories(justaman_cpp20 ${LINK_TYPE}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp20/include>
) # tell cmake to not cache this dir when someone links this 
#   (so cmake doesnt think the folder lives in the same directory on all projects its included)

target_compile_options(justaman_cpp20 INTERFACE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra> # if debug mode, disable optimizations and enable stricter warnings
    $<$<CONFIG:Release>:-O3 -DNDEBUG> # if release, disable debug features and enable agressive optimiations
)

#######################################################################################
# ------------------------------ C++23 SPECIFIC BUILDS ----------------------------- #
#######################################################################################

file(GLOB_RECURSE CPP23_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp23/src/*.cpp")

if(NOT CPP23_SRCS)
    add_library(justaman_cpp23 INTERFACE)
    set(LINK_TYPE INTERFACE)
else()
    add_library(justaman_cpp23 STATIC ${CPP23_SRCS})
    set(LINK_TYPE PUBLIC)
endif()

add_library(justaman::cpp23 ALIAS justaman_cpp23)
target_compile_features(justaman_cpp23 ${LINK_TYPE} cxx_std_23)
target_include_directories(justaman_cpp23 ${LINK_TYPE}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp23/include>
)

target_compile_options(justaman_cpp23 INTERFACE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # fix the MinGW / std::print linker error:
    target_link_libraries(justaman_cpp23 INTERFACE "stdc++exp")
endif()

# chain them
target_link_libraries(justaman_cpp23 ${LINK_TYPE} justaman_cpp20)

#######################################################################################
# ------------------------------ C++26 SPECIFIC BUILDS ----------------------------- #
#######################################################################################

file(GLOB_RECURSE CPP26_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp26/src/*.cpp")

if(NOT CPP26_SRCS)
    add_library(justaman_cpp26 INTERFACE)
    set(LINK_TYPE INTERFACE)
else()
    add_library(justaman_cpp26 STATIC ${CPP26_SRCS})
    set(LINK_TYPE PUBLIC)
endif()

add_library(justaman::cpp26 ALIAS justaman_cpp26)
target_compile_features(justaman_cpp26 ${LINK_TYPE} cxx_std_26)
target_include_directories(justaman_cpp26 ${LINK_TYPE}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp26/include>
)

target_compile_options(justaman_cpp26 INTERFACE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# chain them
target_link_libraries(justaman_cpp26 ${LINK_TYPE} justaman_cpp23)