cmake_minimum_required(VERSION 2.8)
project(Euluna)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -O0 -g -Wno-unused-parameter")
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/src)
add_executable(tests
tests/tests.cpp)
target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} lua)
add_test(AllEulunaTests tests)
