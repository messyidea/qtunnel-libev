cmake_minimum_required(VERSION 3.3)
project(qtunnel_c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES qtunnel.c)
add_executable(qtunnel_c ${SOURCE_FILES})