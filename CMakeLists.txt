cmake_minimum_required(VERSION 3.21)
project(QtOpenGLViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGl OpenGLWidgets)
qt_standard_project_setup()

qt_add_executable(viewer
    src/main.cpp
    src/window.h    src/window.cpp
    src/widget.h    src/widget.cpp
)

target_link_libraries(viewer PRIVATE 
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    )