file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
add_executable(TESTING_BINARY ${SRC_FILES})

target_compile_features(TESTING_BINARY INTERFACE cxx_std_26)
target_link_libraries(TESTING_BINARY PRIVATE justaman::cpp26)