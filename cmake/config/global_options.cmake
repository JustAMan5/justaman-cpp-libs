include_guard(GLOBAL)

option(JUSTAMAN_EXTRA_LOGGING             "Logs each step during configuration in the output window"        OFF)
option(JUSTAMAN_TIME_PROFILING            "Profiles and shows the time it takes for each phase during compilation (only if the compiler supports it)"        
                                                                                                            OFF) # NOT IMPLEMENTED

option(JUSTAMAN_FETCH_GOOGLE_BENCHMARK_LIB "Downloads the C++ google benchmark library for benchmarking (if its a top level project only)"
                                                                                                            OFF)                                                                                                            

option(JUSTAMAN_WARNINGS_AS_ERRORS        "Treat compiler warnings as hard compilation failures"            ON)
option(JUSTAMAN_EXTRA_WARNINGS            "Enables flags like -Wall -Wextra -Wpedantic for GCC and Clang"   ON)

option(JUSTAMAN_HARDWARE_OPTIMIZATIONS    "Enable native CPU hardware supported optimizations"              OFF)
option(JUSTAMAN_LINK_TIME_OPT             "Enable Link Time Optimizations"                                  ON)

option(JUSTAMAN_DISABLE_EXCEPTIONS        "Compile libraries without C++ exceptions"                        ON)
option(JUSTAMAN_DISABLE_RTTI              "Disable RTTI (Runtime type infomration, typeid, dynamic_cast)"   OFF)
option(JUSTAMAN_ENABLE_FAST_MATH          "Enable aggressive floating-point optimizations (-ffast-math)"    OFF)
option(JUSTAMAN_OPTIMIZE_FOR_SIZE         "Optimize binary size (-Os) instead of speed (-O3) in Release"    OFF)
