cmake_minimum_required(VERSION 3.16)

project(crash-organizer-service VERSION 0.1 LANGUAGES CXX)

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

    qt_add_executable(crash-organizer-service
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        cos.h
        cosec.h
    )

target_link_libraries(crash-organizer-service PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


include(GNUInstallDirs)
install(TARGETS crash-organizer-service
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(crash-organizer-service)
endif()
