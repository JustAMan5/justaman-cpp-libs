include_guard(GLOBAL)

include(CheckCompilerFlag)

# only useful for passing the properties to anything that links to it
add_library(justaman_compiler_profile INTERFACE)


###################################################################################################################
# ------------------------------------------ BASE FLAGS, CONFIGS AND SUCH ----------------------------------------
#################################################################################################################

target_compile_options(justaman_compiler_profile INTERFACE
    # ---------- gcc & clang toolchain
    $<$<CXX_COMPILER_ID:Clang,GNU>: # if the compiler is clang or gcc
        $<$<BOOL:${JUSTAMAN_WARNINGS_AS_ERRORS}>:-Werror>
        $<$<BOOL:${JUSTAMAN_EXTRA_WARNINGS}>: -Wall -Wextra -Wpedantic -Wshadow -Wconversion>
        $<$<CONFIG:Release>: # if in release
            $<$<BOOL:${JUSTAMAN_OPTIMIZE_FOR_SIZE}>:-Os> # optimize for size
            $<$<NOT:$<BOOL:${JUSTAMAN_OPTIMIZE_FOR_SIZE}>>:-O3 -DNDEBUG> # default agressive opt release
        >
        $<$<CONFIG:Debug>:-g -O0>
    >
    # ---------- msvc toolchain
    $<$<CXX_COMPILER_ID:MSVC>: # if the compiler is msvc
        /Zc:__cplusplus
        $<$<BOOL:${JUSTAMAN_WARNINGS_AS_ERRORS}>:/WX>
        $<$<BOOL:${JUSTAMAN_EXTRA_WARNINGS}>: /W4 /permissive->
        $<$<CONFIG:Release>: # if in release
            $<$<BOOL:${JUSTAMAN_OPTIMIZE_FOR_SIZE}>:/O1>
            $<$<NOT:$<BOOL:${JUSTAMAN_OPTIMIZE_FOR_SIZE}>>:/O2 /DNDEBUG>
        >
        $<$<CONFIG:Debug>:/Od>
    >
)

###################################################################################################################
# ---------------------------------- OPTIONAL CONFIGS (via global_options.cmake) --------------------------------
#################################################################################################################

# ----------------------- FAST MATH
if(JUSTAMAN_ENABLE_FAST_MATH)
    target_compile_options(justaman_compiler_profile INTERFACE 
        $<$<CXX_COMPILER_ID:Clang,GNU>:-ffast-math> 
        $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>
    )
endif()

# ----------------------- disable exceptions
if(JUSTAMAN_DISABLE_EXCEPTIONS)
    target_compile_options(justaman_compiler_profile INTERFACE 
        $<$<CXX_COMPILER_ID:Clang,GNU>:-fno-exceptions>
        $<$<CXX_COMPILER_ID:MSVC>:/EHs-c->
    )
endif()

# ----------------------- disable RTTI
if(JUSTAMAN_DISABLE_RTTI)
    target_compile_options(justaman_compiler_profile INTERFACE 
        $<$<CXX_COMPILER_ID:Clang,GNU>:-fno-rtti>
        $<$<CXX_COMPILER_ID:MSVC>:/GR->
    )
endif()

# ----------------- native hardware optimizations
if(JUSTAMAN_HARDWARE_OPTIMIZATIONS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        check_compiler_flag(CXX "-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
        
        if(COMPILER_SUPPORTS_MARCH_NATIVE)
            target_compile_options(justaman_compiler_profile INTERFACE "-march=native")
        endif()

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # MSVC doesnt have any -march=native flag, so fallback to explicit instructions support
        check_compiler_flag(CXX "/arch:AVX512" COMPILER_SUPPORTS_AVX512)
        check_compiler_flag(CXX "/arch:AVX2" COMPILER_SUPPORTS_AVX2)

        if(COMPILER_SUPPORTS_AVX512)
            target_compile_options(justaman_compiler_profile INTERFACE "/arch:AVX512")
        elseif(COMPILER_SUPPORTS_AVX2)
            target_compile_options(justaman_compiler_profile INTERFACE "/arch:AVX2")
        endif()
    endif() 
endif()

# ---------------- linke time optimization (LTO)
if(JUSTAMAN_LTO_FLAG)
    target_compile_options(justaman_compiler_profile INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto>
        $<$<CXX_COMPILER_ID:MSVC>:/GL>
    )
    target_link_options(justaman_compiler_profile INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:-flto=thin>
        $<$<CXX_COMPILER_ID:GNU>:-flto=auto>
        $<$<CXX_COMPILER_ID:MSVC>:/LTCG>
    )
endif()


#[===[

gonna make a guide for myself for all the flags here cuz 
rn im not too smart to remember them all or just know what they do from their name

------------ GCC & CLANG:

    * "-g"            -> at cost of speed and size, it helps to see clearly for an human errors and where they happen, so others debugging tools like valgrind can guide u to the exact line of code of a program crash
    * "-O0"           -> Tells GCC and Clang to varely optimize ur code, so its almost like a line to line interpreter
    * "-Werror"       -> treat all compiler warnings as critical compilation errors.
    * "-Wall"         -> enables some optional extra warnings that r not necessary but very useful (e.g. warn for uninitialized variables).
    * "-Wextra"       -> enables more stricter warning that are not covered for -Wall (e.g. unused parameters, signs in comparisons).
    * "-Wpedantic"    -> forces the strict rules of the standard C/C++ code, if ignored, warning will show (e.g. initializing an array like int arr[0] or int v;;;; will throw a warning with this flag, else ignored)
    * "-Wshadow"      -> warns if a variable in an inner scope has the same name as a different variable outside the scope ("shadowing")
    * "-Wconversion"  -> warns any implicit type conversions that could silently lose data (e.g., assigning a double to an int).
    * "-Os"           -> optimizes code for the smallest binary size possible, sacrificing speed if necessary.
    * "-O3"           -> aggressive compiler optimizations. Rewrites code heavily for maximum runtime execution speed.
    * "-DNDEBUG"      -> preprocessor macro that disables standard `assert()` assertions and standard debugging logic.
    * "-ffast-math"   -> optimizes math operations to run faster, especially on floating point numbers (assuming NaN/INF is never possible, no checks for overflows, remove strict rules for floating points).
    * "-fno-exceptions"-> disables C++ exception handling (try, catch, throw). Code compiles smaller and faster without stack unwinding overhead.
    * "-fno-rtti"     -> disables runtime type information (dynamic_cast, typeid). Reduces binary size metadata.
    * "-march=native" -> detects host machine's CPU features (AVX2, AVX512, etc.) and optimizes instructions exclusively for it.
    * "-flto=thin" (clang)-> Link-Time Optimization. Enables global optimizations across all files (like inlining between different .cpp files and such). "Thin" runs fast and in parallel.
    * "-flto=auto" (gcc)-> Link-Time Optimization. same as -flto=thin in clang, =auto means the amount of threads to use to make faster this process, =auto is autoselected and for exemple =4 is 4 threads


-------------- MSVC:

    * "/Od"           -> disable all optimizations, equivalent to -O0
    * "/WX"           -> treat all compiler warnings as critical compilation errors (same as -Werror in GCC and Clang).
    * "/W4"           -> production-quality warning level. Catches structural errors and informational mistakes.
    * "/permissive-"  -> forces the strict C/C++, similar to -Wpedantic in GCC
    * "/Zc:__cplusplus"-> Forces the __cplusplus to correctly say the standard year (e.g. 201703L) rather than an old hardcoded legacy value.
    * "/O1"           -> equivalent to -Os in GCC/Clang.
    * "/O2"           -> Optimizes code for maximum execution speed (Aggressive optimization mode for MSVC).
    * "/DNDEBUG"      -> same as -DNDEBUG
    * "/fp:fast"      -> same as -ffast-math
    * "/EHs-c-"       -> disables C++ exceptions (equivalent to -fno-exceptions)
    * "/GR-"          -> disables runtime type information (equivalent to -fno-rtti).
    * "/arch:AVX2"    -> generates vector instructions optimized for modern CPUs that support advanced vector extensions 2.
    * "/arch:AVX512"  -> generates intensive vector processing instructions for high-end modern CPUs supporting AVX-512.
    * "/GL"           -> whole Program Optimization. Tells the compiler to leave parsing open so code optimization can happen during linking.
    * "/LTCG"         -> Link-Time Code Generation. Used at the link step to execute the actual optimizations enabled by `/GL`.


]===]