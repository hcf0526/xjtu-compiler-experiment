add_executable(gui
    compiler_window.cpp
    compiler_signal.cpp
)

qt_add_resources(RESOURCE_FILES resources.qrc)
target_sources(gui PRIVATE ${RESOURCE_FILES})

target_include_directories(gui PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(gui PRIVATE Qt6::Widgets)
target_link_libraries(gui PRIVATE basic)