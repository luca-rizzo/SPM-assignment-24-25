cmake_minimum_required(VERSION 3.29)
project(SPM_assignment)

set(CMAKE_CXX_STANDARD 14)

include_directories(assigment1/include)

add_executable(SPM_assignment
    assigment1/include/avx_mathfun.h
    assigment1/include/hpc_helpers.hpp
    assigment1/Makefile
    assigment1/softmax_auto.cpp
    assigment1/softmax_avx.cpp
    assigment1/softmax_plain.cpp
)
