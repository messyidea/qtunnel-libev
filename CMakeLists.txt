cmake_minimum_required(VERSION 3.3)
project(qtunnel_c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PkgConfig REQUIRED)
pkg_search_module(LIBSODIUM REQUIRED libsodium)

include_directories(${LIBSODIUM_INCLUDE_DIRS})
link_directories(${LIBSODIUM_LIBRARY_DIRS})
add_definitions(${LIBSODIUM_CFLAGS_OTHER})

set(SOURCE_FILES qtunnel.c)
add_executable(qtunnel_c ${SOURCE_FILES})

target_link_libraries(qtunnel_c ${LIBSODIUM_LIBRARIES})

#see http://stackoverflow.com/questions/27514291/how-to-add-compiler-arguments-using-cmake