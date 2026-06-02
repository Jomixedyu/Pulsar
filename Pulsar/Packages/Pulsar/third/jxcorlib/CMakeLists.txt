
cmake_minimum_required (VERSION 3.8)

project ("jxcorlib")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options("-g")
endif()

option(${PROJECT_NAME}_BUILD_EXAMPLE "example" OFF)
set(CMAKE_CXX_STANDARD 23)

include_directories(".")

add_subdirectory("./CoreLib")
add_subdirectory("./CoreLib.Serialization")
add_subdirectory("./CoreLib.Math")
add_subdirectory("./CoreLib.Platform")

if(${PROJECT_NAME}_BUILD_EXAMPLE)
	add_subdirectory("./Example")
	add_executable (${PROJECT_NAME} "JxCode.CoreLib.cpp")
	target_link_libraries(${PROJECT_NAME} "Example")
else()
	add_library(${PROJECT_NAME} STATIC "lib.cpp")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "./")

target_link_libraries(${PROJECT_NAME} PUBLIC "jxcorlib.core")
target_link_libraries(${PROJECT_NAME} PUBLIC "jxcorlib.ser")
target_link_libraries(${PROJECT_NAME} PUBLIC "jxcorlib.math")
target_link_libraries(${PROJECT_NAME} PUBLIC "jxcorlib.platform")
