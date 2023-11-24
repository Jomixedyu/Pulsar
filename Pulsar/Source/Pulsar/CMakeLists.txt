
cmake_minimum_required (VERSION 3.8)
project("Pulsar")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

option(${PROJECT_NAME}_BUILD_EXECUTABLE "enable executable mode" OFF)
#set(${PROJECT_NAME}_BUILD_EXECUTABLE ON)

file(GLOB_RECURSE Pulsar_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")

add_subdirectory("./third/jxcorlib")
add_subdirectory("./third/gfx")
add_subdirectory("./third/gfx-vk")
add_subdirectory("./third/imgui")

set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
add_subdirectory("./third/glfw")


if(${PROJECT_NAME}_BUILD_EXECUTABLE)
	add_executable(${PROJECT_NAME} ${Pulsar_SRC} "main.cpp")
else()
	add_library(${PROJECT_NAME} STATIC ${Pulsar_SRC})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "./include")

target_include_directories(${PROJECT_NAME} PRIVATE "./include/Pulsar")
target_include_directories(${PROJECT_NAME} PUBLIC "..")

target_compile_definitions(${PROJECT_NAME} PUBLIC PULSAR_GFX_BUILD_VULKAN)

target_link_libraries(${PROJECT_NAME} 
PUBLIC
	jxcorlib
	glfw
	gfx
	gfx-vk
	imgui
)
