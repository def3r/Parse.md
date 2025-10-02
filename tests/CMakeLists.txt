cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
find_package(GTest REQUIRED)

file(GLOB TESTFILES *.cpp)
add_executable(test_markdown ${TESTFILES} ../parse.cpp)
include_directories(${GTEST_INCLUDE_DIRS})
target_include_directories(test_markdown PRIVATE ../)
target_link_libraries(test_markdown GTest::GTest GTest::Main pthread)

add_test(NAME MarkdownTest COMMAND test_markdown)
