cmake_minimum_required(VERSION 3.31)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)
add_subdirectory(basic)

file(GLOB TEST_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/test ${CMAKE_SOURCE_DIR}/test/*)

foreach(test_dir ${TEST_DIRS})
  if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/test/${test_dir}")
    add_subdirectory(test/${test_dir})
  endif()
endforeach()