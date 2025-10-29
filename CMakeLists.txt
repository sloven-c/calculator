cmake_minimum_required(VERSION 4.0)
project(calculator C)

set(CMAKE_C_STANDARD 23)

add_executable(calculator main.c
        structures.h
        stack.c
        stack.h
)
target_link_libraries(calculator m)