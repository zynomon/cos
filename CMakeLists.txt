cmake_minimum_required(VERSION 3.16)
project(trigonometry VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(GNUInstallDirs)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_library(crash SHARED
    CRASH/cos.cpp
    CRASH/cosec.cpp
    CRASH/cosec.h
    CRASH/cos.h
)

add_library(term SHARED
    TERM/tan.cpp
    TERM/tan.h
)

set_target_properties(crash PROPERTIES PREFIX "lib" OUTPUT_NAME "crash")
target_link_libraries(crash PRIVATE Qt6::Core Qt6::Widgets)

set_target_properties(term PROPERTIES PREFIX "lib" OUTPUT_NAME "term")
target_link_libraries(term PRIVATE Qt6::Core)

find_program(STRIP_EXECUTABLE strip)
if(STRIP_EXECUTABLE)
    add_custom_command(TARGET crash term POST_BUILD
        COMMAND ${STRIP_EXECUTABLE} $<TARGET_FILE:crash>
        COMMAND ${STRIP_EXECUTABLE} $<TARGET_FILE:term>
        COMMENT "Stripping files..."
    )
endif()

install(TARGETS crash term
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/trigonometry
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/trigonometry
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    install(FILES CRASH/cos.h CRASH/cosec.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry/crash
    )
    install(FILES CRASH/cos.cpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry/crash
        RENAME cos
    )
    install(FILES CRASH/cosec.cpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry/crash
        RENAME cosec
    )
    install(FILES TERM/tan.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry/term
    )
    install(FILES TERM/tan.cpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry/term
        RENAME tan
    )
    install(FILES TrigConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Trig/
    )
    install(FILES .deb/README.txt
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Trig/
    )
endif()

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "libtrigonometry")
set(CPACK_PACKAGE_VENDOR "error.os")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Contains COS (CXX) + COSEC (Qt) crash handling libraries and TERM library"
)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT "Zynomon Aelius <zynomon@proton.me>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zynomon Aelius")
set(CPACK_GENERATOR "DEB;TGZ;ZIP")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6core6, libqt6gui6, libqt6widgets6")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/.deb/postinst"
    "${CMAKE_CURRENT_SOURCE_DIR}/.deb/prerm"
)

include(CPack)

# use Debug profile <"  cmake --build . --config Debug "> in terminal to install development headers along side.

