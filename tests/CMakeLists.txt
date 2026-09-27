# file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
# add_executable(TESTING_BINARY ${SRC_FILES})

# target_compile_definitions(TESTING_BINARY PRIVATE 
#     PROJECT_SRC_DIR="${CMAKE_SOURCE_DIR}"
# )

# target_compile_features(TESTING_BINARY INTERFACE cxx_std_26)
# target_link_libraries(TESTING_BINARY PRIVATE justaman::cpp26)

# 1. Find all direct test source files (avoiding subdirectories or helper files if needed)
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 2. Iterate over each file to create a dedicated test executable
foreach(SOURCE_FILE ${TEST_SOURCES})
    message(STATUS "FILE PROCESSED ${SOURCE_FILE}")
    # Get the filename without extension (e.g., "test_vector" from "test_vector.cpp")
    get_filename_component(TEST_NAME ${SOURCE_FILE} NAME_WE)
    
    # Define a unique binary name for this specific test
    set(TARGET_NAME "test_${TEST_NAME}")

    # Create the executable for this specific test
    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    # Apply properties and definitions to this specific target
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        PROJECT_SRC_DIR="${CMAKE_SOURCE_DIR}"
    )
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_26) # Changed INTERFACE to PRIVATE for executables
    target_link_libraries(${TARGET_NAME} PRIVATE justaman::cpp26)

    # 3. Register the executable with CTest so it can be run automatically
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})
endforeach()