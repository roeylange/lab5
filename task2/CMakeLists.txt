cmake_minimum_required(VERSION 3.22)
project(task2 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(task2
        LineParser.c
        LineParser.h
        looper.c
        myshell.c)
