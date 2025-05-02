# 搜索 test/lexical/ 目录下所有 .cpp 文件
file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# 遍历每个测试源文件
foreach(test_src ${TEST_SOURCES})
  # 获取不带扩展名的文件名作为目标名字
  get_filename_component(test_name ${test_src} NAME_WE)
  
  # 添加一个可执行程序
  add_executable(${test_name} ${test_src})
  
  # 链接需要的库
  target_link_libraries(${test_name}
      PRIVATE basic
  )
  
  # 包含头文件目录（根据你的项目结构）
  target_include_directories(${test_name}
      PRIVATE ${CMAKE_SOURCE_DIR}
  )
endforeach()
