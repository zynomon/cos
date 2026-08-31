cmake_minimum_required(VERSION 3.16)

project(terminal-angulation-navigator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
test.cpp
)

    qt_add_executable(terminal-angulation-navigator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        
        tan.h
    )

target_link_libraries(terminal-angulation-navigator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


include(GNUInstallDirs)
install(TARGETS terminal-angulation-navigator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(terminal-angulation-navigator)
endif()
