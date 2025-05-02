file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(test_src ${TEST_SOURCES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name}
      PUBLIC basic
  )
  target_include_directories(${test_name}
      PRIVATE ${CMAKE_SOURCE_DIR}
  )
endforeach()
