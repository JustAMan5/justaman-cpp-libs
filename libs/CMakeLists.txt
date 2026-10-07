include_guard(GLOBAL)

include(compiler_flags)

#######################################################################################
# ------------------------------ C++20 SPECIFIC BUILDS ----------------------------- #
#######################################################################################
file(GLOB_RECURSE CPP20_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp20/src/*.cpp")

if(NOT CPP20_SRCS)
    add_library(justaman_cpp20 INTERFACE) # mark the library as interface (no binary needed, only forwards properties when linked)
    set(LINK_TYPE_CXX20 INTERFACE)
else()
    add_library(justaman_cpp20 STATIC ${CPP20_SRCS})
    set(LINK_TYPE_CXX20 PUBLIC)
endif()

add_library(justaman::cpp20 ALIAS justaman_cpp20)

set_target_properties(justaman_cpp20 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(justaman_cpp20 ${LINK_TYPE_CXX20}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp20/include>
) # tell cmake to not cache this dir when someone links this 
#   (so cmake doesnt think the folder lives in the same directory on all projects its included)

target_link_libraries(justaman_cpp20 ${LINK_TYPE_CXX20} justaman_compiler_profile)

#######################################################################################
# ------------------------------ C++23 SPECIFIC BUILDS ----------------------------- #
#######################################################################################

file(GLOB_RECURSE CPP23_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp23/src/*.cpp")

if(NOT CPP23_SRCS)
    add_library(justaman_cpp23 INTERFACE)
    set(LINK_TYPE_CXX23 INTERFACE)
else()
    add_library(justaman_cpp23 STATIC ${CPP23_SRCS})
    set(LINK_TYPE_CXX23 PUBLIC)
endif()

add_library(justaman::cpp23 ALIAS justaman_cpp23)

set_target_properties(justaman_cpp23 PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(justaman_cpp23 ${LINK_TYPE_CXX23}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp23/include>
)

target_link_libraries(justaman_cpp23 ${LINK_TYPE_CXX23} justaman_compiler_profile)

if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # fix the MinGW / std::print linker error:
    target_link_libraries(justaman_cpp23 ${LINK_TYPE_CXX23} "stdc++exp")
endif()

# chain them
target_link_libraries(justaman_cpp23 ${LINK_TYPE_CXX23} justaman_cpp20)

#######################################################################################
# ------------------------------ C++26 SPECIFIC BUILDS ----------------------------- #
#######################################################################################

file(GLOB_RECURSE CPP26_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp26/src/*.cpp")

if(NOT CPP26_SRCS)
    add_library(justaman_cpp26 INTERFACE)
    set(LINK_TYPE_CXX26 INTERFACE)
else()
    add_library(justaman_cpp26 STATIC ${CPP26_SRCS})
    set(LINK_TYPE_CXX26 PUBLIC)
endif()

add_library(justaman::cpp26 ALIAS justaman_cpp26)

set_target_properties(justaman_cpp26 PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
)
target_compile_features(justaman_cpp26 ${LINK_TYPE_CXX26} cxx_std_26)
target_include_directories(justaman_cpp26 ${LINK_TYPE_CXX26}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp26/include>
)

target_link_libraries(justaman_cpp26 ${LINK_TYPE_CXX26} justaman_compiler_profile)

# chain them
target_link_libraries(justaman_cpp26 ${LINK_TYPE_CXX26} justaman_cpp23)