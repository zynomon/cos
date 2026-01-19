cmake_minimum_required(VERSION 3.16)

project(trigonometry VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_library(cos SHARED cos.cpp cos.h)
target_link_libraries(cos PRIVATE Qt6::Core Qt6::Widgets)

add_library(cosec SHARED cosec.cpp cosec.h)
target_link_libraries(cosec PRIVATE Qt6::Core Qt6::Widgets)

install(TARGETS cos cosec
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/trigonometry
)

install(FILES cos.h cosec.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry
)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "trigonometry")
set(CPACK_PACKAGE_VENDOR "error.os")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    " lib trigonometry is a library"
)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CPACK_PACKAGE_CONTACT "Zynomon Aelius <zynomon@proton.me>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zynomon Aelius")

set(CPACK_GENERATOR "DEB;TGZ;ZIP")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6core6, libqt6gui6, libqt6widgets6")

include(CPack)

