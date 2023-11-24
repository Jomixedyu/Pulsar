cmake_minimum_required(VERSION 3.21)
project(imgui)

set(_imgui_Sources
        include/imgui/imconfig.h
        src/imgui.cpp
        include/imgui/imgui.h
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        include/imgui/imgui_internal.h
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        include/imgui/imgui_impl_vulkan.h
        src/imgui_impl_vulkan.cpp
        include/imgui/imgui_impl_glfw.h
        src/imgui_impl_glfw.cpp
        include/imgui/imgui_impl_win32.h
        src/imgui_impl_win32.cpp
        LICENSE.txt
)
include_directories("../glfw/include")

add_library(${PROJECT_NAME} STATIC ${_imgui_Sources})
#target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/imgui")

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})