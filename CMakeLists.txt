option(PARSEMD_DEBUG "Set to ON for debug build" OFF)
option(PARSEMD_BUILD_TESTS "Set to ON to build tests" OFF)
option(PARSEMD_DEV_WARNINGS "Enable compiler warnings and warnings as errors" OFF)
option(PARSEMD_BUILD_BENCHMARK "Set to ON to build benchmark" OFF)

cmake_minimum_required(VERSION 3.16)

project(parsemd
  LANGUAGES   CXX
  VERSION     0.1
  DESCRIPTION "C++ Markdown Parser"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(parsemd STATIC)

target_include_directories(parsemd
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (PARSEMD_DEBUG)
  target_compile_options(parsemd PRIVATE -g)
else()
  target_compile_options(parsemd PRIVATE -O3)
endif()
if (PARSEMD_DEV_WARNINGS)
  target_compile_options(parsemd PRIVATE -Wall -Wextra)
endif()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/parsemd/*.cpp)
target_sources(parsemd
  PRIVATE
    ${SOURCES}
)

if (PARSEMD_BUILD_TESTS)
  enable_testing()
  add_subdirectory(./tests)
endif()

if (PARSEMD_BUILD_BENCHMARK)
  add_subdirectory(./benchmark)
endif()
