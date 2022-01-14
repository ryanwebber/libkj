cmake_minimum_required(VERSION 3.4)
project("LibKJ" CXX)
set(VERSION 0.10-dev)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Preventing an in-tree build.")
endif()

add_subdirectory(src)

