include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(htmlqt_SRC
    htmlabstractphase.cpp
    htmlinitialphase.cpp
    htmlbeforehtmlphase.cpp
    htmltokenizer.cpp
    htmltokenizer_p.h
    htmlparser.cpp
    htmlparser_p.h
    htmltree.cpp
)

set(htmlqt_HEADERS
    htmltokenizer.h
    htmlparser.h
    htmltree.h
)

# set(htmlqt_HEADERS_PRIVATE
#     common.h
# )

add_definitions(
    -std=c++11
)

add_library(html-qt5 SHARED ${htmlqt_SRC} ${htmlqt_HEADERS} ${htmlqt_HEADERS_PRIVATE})
set_target_properties(html-qt5 PROPERTIES VERSION ${HTMLQT_VERSION} SOVERSION ${HTMLQT_API_LEVEL})

qt5_use_modules(html-qt5 Core Network)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/html-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/html-qt5.pc
  @ONLY
)

install(TARGETS html-qt5 EXPORT HTMLQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/html-qt5.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${htmlqt_HEADERS}
        DESTINATION include/html-qt5/HTMLQt
)
