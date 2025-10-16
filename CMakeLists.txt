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

target_compile_options(parsemd
  PRIVATE
    -Wall -Wextra -g
)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/parsemd/*.cpp)
target_sources(parsemd
  PRIVATE
    ${SOURCES}
)

enable_testing()
add_subdirectory(./tests)
