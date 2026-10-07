include_guard(GLOBAL)

include(FetchContent)
include(global_options)
include(network_utils)

macro(smart_fetch_dependency DEP_NAME)
    # tbh i dont know what this three lines do, but they make the interface cool, so fuck it
    set(options)
    set(oneValueArgs REPO_URL REPO_TAG LOCAL_PATH FIND_PACKAGE_NAME)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_FIND_PACKAGE_NAME)
        set(ARG_FIND_PACKAGE_NAME ${DEP_NAME})
    endif()

    # check if the system package manager already has this target, quiet means it doesnt fail with a critical when is not found
    find_package(${ARG_FIND_PACKAGE_NAME} QUIET)
    
    if(${ARG_FIND_PACKAGE_NAME}_FOUND)
        if(JUSTAMAN_EXTRA_LOGGING)
            message(STATUS "Dependency [${DEP_NAME}] already resolved via system/package manager.")
        endif()
    else()
        check_internet_connectivity()

        if(JUSTAMAN_HAS_INTERNET AND ARG_REPO_URL)
            if(JUSTAMAN_EXTRA_LOGGING)
                message(STATUS "Downloading [${DEP_NAME}] (${ARG_REPO_TAG}) from remote repository...")
            endif()

            FetchContent_Declare(
                ${DEP_NAME}
                GIT_REPOSITORY ${ARG_REPO_URL}
                GIT_TAG        ${ARG_REPO_TAG}

                SOURCE_DIR     "${PROJECT_SOURCE_DIR}/vendor/src/${DEP_NAME}"   
                BINARY_DIR     "${PROJECT_SOURCE_DIR}/vendor/build/${DEP_NAME}"
            )

            FetchContent_MakeAvailable(${DEP_NAME})
        else()
            # offline, using our cleanly structured vendor path
            if(JUSTAMAN_EXTRA_LOGGING)
                message(STATUS "Offline/Local Mode: Searching for local fallback for [${DEP_NAME}]...")
            endif()
            
            if(EXISTS "${ARG_LOCAL_PATH}" AND EXISTS "${ARG_LOCAL_PATH}/CMakeLists.txt")

                FetchContent_Declare(
                    ${DEP_NAME}
                    SOURCE_DIR "${ARG_LOCAL_PATH}"
                    # avoid cmake to update or download anything
                    DOWNLOAD_COMMAND "" 
                    UPDATE_COMMAND ""
                )

                FetchContent_MakeAvailable(${DEP_NAME})

            else()
                message(FATAL_ERROR "CRITICAL: Cannot find dependency [${DEP_NAME}] because of offline mode or failing to connect to github.")
            endif()
        endif()
    endif()
endmacro()