cmake_minimum_required(VERSION 3.22)
project(Lab21 C)

set(CMAKE_C_STANDARD 17)

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

add_executable(Lab21 main.c)
