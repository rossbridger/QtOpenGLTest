cmake_minimum_required(VERSION 3.5)

project(QtOpenGLTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_VERSION_MAJOR 6)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL)
# this is heuristically generated, and may not be correct
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtOpenGLTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
		openglwidget.h openglwidget.cpp
		mesh.h mesh.cpp
		model.h model.cpp
		model.vs
		model.fs
		screen_texture.vs
		screen_texture.fs
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET QtOpenGLTest APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(QtOpenGLTest SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(QtOpenGLTest
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(QtOpenGLTest PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(QtOpenGLTest PRIVATE assimp::assimp Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGLWidgets Qt${QT_VERSION_MAJOR}::OpenGL)

set_target_properties(QtOpenGLTest PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QtOpenGLTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtOpenGLTest)
endif()
