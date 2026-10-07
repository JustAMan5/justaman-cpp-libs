include_guard(GLOBAL)

include(global_options)
include(fetch_helpers)

set(JUSTAMAN_DEPENDENCIES)


if(PROJECT_IS_TOP_LEVEL AND JUSTAMAN_FETCH_GOOGLE_BENCHMARK_LIB)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "") # avoid compiling their own tests
    smart_fetch_dependency(benchmark
        REPO_URL          "https://github.com/google/benchmark.git"
        REPO_TAG          "v1.9.5"
        LOCAL_PATH        "${PROJECT_SOURCE_DIR}/vendor/src/benchmark" # todo to check
    )
    set(JUSTAMAN_DEPENDENCIES ${JUSTAMAN_DEPENDENCIES} benchmark::benchmark benchmark::benchmark_main)
endif()