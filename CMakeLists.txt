cmake_minimum_required(VERSION 3.16)

project(trigonometry VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets) # Qt
# cos;
add_library(cos SHARED cos.cpp cos.h)
target_link_libraries(cos PRIVATE Qt6::Core Qt6::Widgets)
# cosec;
add_library(cosec SHARED cosec.cpp cosec.h)
target_link_libraries(cosec PRIVATE Qt6::Core Qt6::Widgets)


find_program(STRIP_EXECUTABLE strip)
if(STRIP_EXECUTABLE)
    add_custom_command(TARGET cos POST_BUILD
        COMMAND ${STRIP_EXECUTABLE} $<TARGET_FILE:cos>
    )
    add_custom_command(TARGET cosec POST_BUILD
        COMMAND ${STRIP_EXECUTABLE} $<TARGET_FILE:cosec>
    )
endif()

install(TARGETS cos cosec
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/trigonometry
)

# use Debug profile <"  cmake --build . --config Debug   "> in terminal
if(CMAKE_BUILD_TYPE STREQUAL "Debug" )
    install(FILES cos.h cosec.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry
    )
    install(DIRECTORY COS COSEC
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trigonometry
        FILES_MATCHING PATTERN "*.h"
    )
endif()




# .deb, tgz and zip
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "libtrigonometry")
set(CPACK_PACKAGE_VENDOR "error.os")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Contains COS + COSEC (Qt) crash handling libraries"
)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CPACK_PACKAGE_CONTACT "Zynomon Aelius <zynomon@proton.me>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zynomon Aelius")

set(CPACK_GENERATOR "DEB;TGZ;ZIP")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6core6, libqt6gui6, libqt6widgets6")

include(CPack)
