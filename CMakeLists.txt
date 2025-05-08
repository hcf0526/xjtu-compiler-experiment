cmake_minimum_required(VERSION 3.31)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "D:/Software/Qt/6.9.0/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_subdirectory(utils)
add_subdirectory(basic)
add_subdirectory(gui)

file(GLOB TEST_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/test ${CMAKE_SOURCE_DIR}/test/*)

foreach(test_dir ${TEST_DIRS})
  if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/test/${test_dir}")
    add_subdirectory(test/${test_dir})
  endif()
endforeach()