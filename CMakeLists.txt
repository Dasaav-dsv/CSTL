cmake_minimum_required(VERSION 3.20.0)
project(CSTL VERSION 1.2.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(CSTL STATIC
    "lib/type.c"
    "lib/vector.c"
    "lib/xstring.c"
)
