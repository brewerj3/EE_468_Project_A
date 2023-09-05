cmake_minimum_required(VERSION 3.27)
project(EE_468_Project_A C)

set(CMAKE_C_STANDARD 99)

add_executable(sshell sshell.c)

add_executable(test test.c)

add_executable(pipe pipe.c)

add_executable(remake shell2.c)

file(COPY almamater DESTINATION ${CMAKE_BINARY_DIR})