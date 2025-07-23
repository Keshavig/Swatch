cmake_minimum_required(VERSION 3.10)
project(SWATCH LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC
    src/main.cpp
    src/convert.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    # imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

include_directories(
    imgui
    imgui/backends
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

if(NOT GLFW_FOUND)
    message(FATAL_ERROR "GLFW not found!")
endif()

include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})
add_executable(swatch ${SRC})

target_link_libraries(swatch
    ${GLFW_LIBRARIES}
    GL
    pthread
)

if(UNIX)
    target_link_libraries(swatch m)
endif()

target_compile_definitions(swatch PRIVATE "IMGUI_IMPL_OPENGL_LOADER_GLAD")
if(DECORATIONS_ENABLE)
    add_compile_definitions(DECORATIONS_ENABLE)
endif()
