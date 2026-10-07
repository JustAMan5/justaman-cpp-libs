include_guard(GLOBAL)

include(global_options)

# remove the variable from the cache so every new unique configuration is processed once
unset(JUSTAMAN_HAS_INTERNET CACHE)

function(check_internet_connectivity)
    if(DEFINED JUSTAMAN_HAS_INTERNET)
        return()
    endif()

    if(JUSTAMAN_EXTRA_LOGGING)
        message(STATUS "Checking internet connectivity for FetchContent...")
    endif()
    
    # try connecting to github with a fast 2-second timeout

    file(DOWNLOAD "https://github.com" "${CMAKE_BINARY_DIR}/.net_probe"
        TIMEOUT 2
        STATUS DOWNLOAD_STATUS
    )

    # clean up the file after the test
    if(EXISTS "${CMAKE_BINARY_DIR}/.net_probe")
        file(REMOVE "${CMAKE_BINARY_DIR}/.net_probe")
    endif()

    # the status list format is: {status_code, status_string}
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)

    if(STATUS_CODE EQUAL 0)
        set(JUSTAMAN_HAS_INTERNET ON CACHE INTERNAL "Internet connectivity status")
        if(JUSTAMAN_EXTRA_LOGGING)
            message(STATUS "Internet connection detected. GitHub is reachable.")
        endif()
    else()
        set(JUSTAMAN_HAS_INTERNET OFF CACHE INTERNAL "Internet connectivity status")
        if(JUSTAMAN_EXTRA_LOGGING)
            list(GET DOWNLOAD_STATUS 1 STATUS_MESSAGE)
            message(WARNING "No internet connection detected or GitHub is unreachable. Reason: ${STATUS_MESSAGE}. Forcing offline fallback mode.")
        endif()
    endif()
endfunction()