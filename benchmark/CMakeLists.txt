cmake_minimum_required(VERSION 3.16)
project(parsemd_benchmark)

find_package(benchmark REQUIRED)
include_directories(${benchmark_INCLUDE_DIRS} ../include/)

add_custom_target(
  RandomData ALL
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/randomData.py ${CMAKE_CURRENT_BINARY_DIR}/data.md
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(bmark bmark.cpp)
target_link_libraries(bmark benchmark::benchmark parsemd)
