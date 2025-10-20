cmake_minimum_required(VERSION 3.10)
project(parsemd_tests)

set(CMAKE_CXX_STANDARD 17)
find_package(GTest REQUIRED)

file(GLOB TESTFILES *.cpp)
add_executable(parsemd_tests ${TESTFILES})
include_directories(${GTEST_INCLUDE_DIRS} ../include/)

target_link_libraries(parsemd_tests GTest::GTest GTest::Main parsemd pthread)

add_test(NAME MarkdownTest COMMAND parsemd_tests)
