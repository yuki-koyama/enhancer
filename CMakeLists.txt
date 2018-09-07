cmake_minimum_required(VERSION 3.1)

project(enhancer CXX)
set(CMAKE_CXX_STANDARD 11)

file(GLOB headers include/enhancer.hpp)

add_library(enhancer INTERFACE)
target_sources(enhancer INTERFACE ${headers})
target_include_directories(enhancer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(FILES ${headers} DESTINATION include)
