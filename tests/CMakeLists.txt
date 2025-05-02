cmake_minimum_required(VERSION 3.20.0)
project(CSTL_tests VERSION 0.1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

add_library(CSTL STATIC
    "../lib/type.c"
    "../lib/vector.c"
)

include(FetchContent)

# v1.17.0
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(CSTL_tests
    "type.cpp"
    "vector.cpp"
)

target_include_directories(CSTL_tests PRIVATE
    "../lib/"
)

target_link_libraries(CSTL_tests
    CSTL
    GTest::gtest_main
)

gtest_discover_tests(CSTL_tests)
