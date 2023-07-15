project(imgui VERSION 1.89)

set(_imgui_Sources
    imconfig.h
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_impl_vulkan.h
    imgui_impl_vulkan.cpp
	imgui_impl_glfw.h
	imgui_impl_glfw.cpp
    LICENSE.txt
)
include_directories("../glfw/include")

add_library(${PROJECT_NAME} STATIC ${_imgui_Sources})
#target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})