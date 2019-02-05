cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
set(CMAKE_CXX_STANDARD 17)

project(DinaturalProject)

include (AddGTest)

add_subdirectory(src/polymorphic_types)
add_subdirectory(src/naturality)

enable_testing()
