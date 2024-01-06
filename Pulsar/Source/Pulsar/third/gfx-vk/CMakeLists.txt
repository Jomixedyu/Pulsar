
project ("gfx-vk")

file(GLOB_RECURSE gfxvk_SRC "include/*.h" "src/*.h" "src/*.cpp")

add_library(${PROJECT_NAME} STATIC OBJECT ${gfxvk_SRC})
add_compile_definitions(VULKAN_FLIPPING_VIEWPORT)
target_include_directories(${PROJECT_NAME} PUBLIC "../gfx/include")
target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/gfx-vk")

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC "GFXThirdParty")
target_link_directories(${PROJECT_NAME} PUBLIC "GFXThirdParty/glfw")
target_link_libraries(${PROJECT_NAME} PUBLIC glfw3_mt)
