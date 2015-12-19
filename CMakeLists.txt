cmake_minimum_required(VERSION 2.8)
project(Euluna)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -O0 -g -Wno-unused-parameter")
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/src)
add_executable(tests
src/eulunabinder.hpp
src/eulunainterface.hpp
src/eulunaengine.hpp
tests/tests.cpp)
target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} lua)
add_custom_target(run COMMAND tests DEPENDS tests WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
add_test(AllEulunaTests tests)
