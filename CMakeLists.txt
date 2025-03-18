cmake_minimum_required(VERSION 3.10)
project(LexerTests)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_markdown test.cpp lexer.cpp)

target_link_libraries(test_markdown GTest::GTest GTest::Main pthread)
