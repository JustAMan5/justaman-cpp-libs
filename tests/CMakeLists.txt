file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(SOURCE_FILE ${TEST_SOURCES})
    # get filename without extensions (.cpp)
    get_filename_component(TEST_NAME ${SOURCE_FILE} NAME_WE)
    
    set(TARGET_NAME "test_${TEST_NAME}")
    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    # apply properties and definitions to this specific target
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        PROJECT_SRC_DIR="${CMAKE_SOURCE_DIR}"
    )
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(${TARGET_NAME} PRIVATE justaman::cpp26)

    # register the executable with ctest so it can be run automatically
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})
endforeach()