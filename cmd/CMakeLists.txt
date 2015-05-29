include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(html_qt_cmd_SRCS
    main.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_executable(html-qt-skell ${html_qt_cmd_SRCS})
qt5_use_modules(html-qt-skell Core)
target_link_libraries(html-qt-skell
    html-qt5
)

set_target_properties(html-qt-skell PROPERTIES OUTPUT_NAME html-qt)
install(TARGETS html-qt-skell DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
