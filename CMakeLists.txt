cmake_minimum_required(VERSION 3.24)
project(os_lab3 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(os_lab3
        ask3.c ask3a.c)
